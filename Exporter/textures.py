import bpy #type: ignore

def bake_texture(bakeType: str, savePath: str, imageheight: int = 1024, imagewidth: int  = 1024):

    #create image and setup its color space
    image = bpy.data.images.new(bakeType, width=imagewidth, height=imageheight)
    if bakeType == "DIFFUSE":
        image.colorspace_settings.name = "sRGB"
    else:
        image.colorspace_settings.name = "Non-Color"

    #get active object material
    object_material = bpy.context.object.active_material
    if not object_material:
        print("Object has no active material")
    object_material.use_nodes = True


    #Node tree nodes and links
    nodeTreeNodes = object_material.node_tree.nodes    
    links = object_material.node_tree.links

    #Image for baking
    imageNode = nodeTreeNodes.new("ShaderNodeTexImage")
    imageNode.image = image
    nodeTreeNodes.active = imageNode
    
    bsdf = nodeTreeNodes.get("Principled BSDF")
    
    try:
        albedo_node = bsdf.inputs["Base Color"].links[0].from_node
        albedo_node_output_socked = bsdf.inputs["Base Color"].links[0].from_socket.name
    
        metalic_node = bsdf.inputs["Metallic"].links[0].from_node
        metalic_node_output_socked = bsdf.inputs["Metallic"].links[0].from_socket.name
        
        roughness_node = bsdf.inputs["Roughness"].links[0].from_node
        roughness_node_output_socked = bsdf.inputs["Roughness"].links[0].from_socket.name
    
    except IndexError:
        print("All nodes must bave input value!")
        nodeTreeNodes.remove(imageNode)
        bpy.data.images.remove(image)
        return None

    #Add Emission shader and link it to material output
    emmitShader = nodeTreeNodes.new(type="ShaderNodeEmission")
    links.new(emmitShader.outputs["Emission"], nodeTreeNodes.get("Material Output").inputs["Surface"])
    bpy.context.scene.render.bake.margin = 3
    #Connect channels from BSDF to emit shader color and bake them
    if bakeType == "DIFFUSE":
        links.new(albedo_node.outputs[albedo_node_output_socked], emmitShader.inputs["Color"])
        bpy.ops.object.bake(type='EMIT')    
    
    if bakeType == "ROUGHNESS":
        links.new(roughness_node.outputs[roughness_node_output_socked], emmitShader.inputs["Color"])
        bpy.ops.object.bake(type='EMIT')    
    
    if bakeType == "METALIC":
        links.new(metalic_node.outputs[metalic_node_output_socked], emmitShader.inputs["Color"])
        bpy.ops.object.bake(type='EMIT')
    #TODO: Fix fucking normals
    if bakeType == "NORMAL":
        bpy.ops.object.bake(type='NORMAL', normal_space='TANGENT', margin=2)

    image.save(filepath = savePath)
    nodeTreeNodes.remove(imageNode)
    nodeTreeNodes.remove(emmitShader)

    #Reconnect bsdf to material output
    links.new(bsdf.outputs["BSDF"], nodeTreeNodes.get("Material Output").inputs["Surface"])
    bpy.data.images.remove(image)