import importlib.abc
import xml.etree.ElementTree as ET
import bpy #type: ignore
from bpy_extras.io_utils import ExportHelper, ImportHelper #type: ignore

import geometry
import shaders

import importlib
importlib.reload(geometry)
importlib.reload(shaders)

#This function is called when selected shader property is called.
#It changes the class that is assigned to SmoothieShaderSettings of currently selected object.
def UpdateShaderWindow(self, context):
    context.object.SmoothieShaderSettings.selected_shader = self.selected_shader
    return None

def getBoundBox(object):
    
    root = ET.Element("BoundingBox")
    min_vec = ET.Element("min")
    min_vec.text = str(round(object.bound_box[4][0], 4)) + " " + str(round(object.bound_box[4][1], 4)) + " " + str(round(object.bound_box[4][2], 4))
    
    max_vec = ET.Element("max")
    max_vec.text = str(round(object.bound_box[2][0], 4)) + " " + str(round(object.bound_box[2][1], 4)) + " " + str(round(object.bound_box[2][2], 4))
    
    root.append(min_vec)
    root.append(max_vec)
    return root

#This class defines property group for each object in scene.
#This properties are used by export scripts to write files to export custom properties for each object
class ModelProperties(bpy.types.PropertyGroup):
    props = bpy.props

    #Scene export settings
    ignore_at_all: props.BoolProperty(name = "Ignore at all", default = False, description = "Model will be ignored by exporter")
    only_update_position: props.BoolProperty(name = "Update local matrix", default = False, description = "Location, scale and rotation of this model will be updated, but model itself wont be changed in any way")

    #Mesh export settings
    export_mesh: props.BoolProperty(name = "Export mesh", default = True, description = "Mesh will be exported at the same location of model file, with name same as name of object in the scene.")
    export_mesh_file: props.StringProperty(name = "Mesh file", description = "Locaton of .sgeometry file")

    #shader export settings
    double_sided: props.BoolProperty(name = "Double sided", default = False, description = "Disables/enables rendering for both sides.")
    useAlpha: props.BoolProperty(name = "Use Alpha", default = False, description = "Disables/enables alpha channel blending")
    
    selected_shader: props.EnumProperty(
        name="Shader", description="Shader that this model will use in render engine", 
        items = shaders.all_shaders, default= shaders.all_shaders[0][0], update = UpdateShaderWindow)

    save_filepath: props.StringProperty(name = "Save filepath", default = "", description = "Filepath where this file should be saved.")


#Import button for selecting .geometry file
class SMOOTHIE_OT_ImportSgeometryFileButton(bpy.types.Operator, ImportHelper):
    bl_idname = "smoothie.import_sgeometry_file_button"
    bl_label = "Select .geometry file"

    filter_glob: bpy.props.StringProperty(default = '*.sgeometry', options = {'HIDDEN'})

    def execute(self, context):
        bpy.context.object.SmoothieExportSettings.export_mesh_file = self.filepath
        return {'FINISHED'}

def WriteModelFile():
    current_object = bpy.context.object
    if(current_object == None):
        print("No object selected")
        return None
    export_settings = current_object.SmoothieExportSettings
    shader_settings = current_object.SmoothieShaderSettings

    
    root = ET.Element("model")
    
    #Geometry file 
    geometryFileElement = ET.SubElement(root, "geometryFile")
    geometryFileLocation = ""
    if export_settings.export_mesh == False:
        geometryFileLocation = str(export_settings.export_mesh_file)
    else:
        geometryFileLocation = export_settings.save_filepath.split(".")[0] + ".sgeometry"
    
    geometry.WriteGeometryFile(geometryFileLocation)
    
    try:
        geometryFileLocation = "resources\\" + geometryFileLocation.split("resources\\")[1]
    except IndexError:
        geometryFileLocation = geometryFileLocation
        print("Warning! Geometry file is not inside \" resources \" folder! Full path will be used.")
        
    geometryFileElement.text = geometryFileLocation
    
    
    #Shader location
    root.append(shader_settings.getShaderLocation())
    
    #Shader properties
    root.append(shader_settings.getPropertyElement())
    
    root.append(getBoundBox(current_object))

    ET.ElementTree(root).write(export_settings.save_filepath)
    return None

class SMOOTHIE_OT_ModelExportButton(bpy.types.Operator, ExportHelper):
    """Button operator that prints to terminal"""
    bl_idname = "smoothie.model_export_button"
    bl_label = "Export Model"

    filename_ext = '.smodel' 
    
    filter_glob: bpy.props.StringProperty(default = '*.smodel', options = {'HIDDEN'})

    
    def execute(self, context):

        bpy.context.object.SmoothieExportSettings.save_filepath = self.filepath
        WriteModelFile()
        
        return {'FINISHED'}
    
def registerClasses():
    for i in shaders.all_shaders_pointers:
        bpy.utils.register_class(i)
    bpy.utils.register_class(shaders.shader)
    

    bpy.utils.register_class(ModelProperties)
    bpy.utils.register_class(SMOOTHIE_OT_ImportSgeometryFileButton) 
    bpy.utils.register_class(SMOOTHIE_OT_ModelExportButton) 
    bpy.types.Object.SmoothieExportSettings = bpy.props.PointerProperty(type=ModelProperties)
    bpy.types.Object.SmoothieShaderSettings = bpy.props.PointerProperty(type=shaders.shader)

def unregisterClasses():
    for i in shaders.all_shaders_pointers:
        bpy.utils.unregister_class(i)
    bpy.utils.unregister_class(shaders.shader)


    bpy.utils.unregister_class(SMOOTHIE_OT_ImportSgeometryFileButton) 
    bpy.utils.unregister_class(SMOOTHIE_OT_ModelExportButton) 
    del bpy.types.Object.SmoothieExportSettings
    del bpy.types.Object.SmoothieShaderSettings
    bpy.utils.unregister_class(ModelProperties)