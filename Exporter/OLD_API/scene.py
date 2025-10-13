import bpy #type: ignore
import xml.etree.ElementTree as ET
import mathutils #type: ignore
from bpy_extras.io_utils import ExportHelper, ImportHelper #type: ignore
import os
import model as model_script
import importlib
importlib.reload(model_script)
import math

#Similar to model export class, only this one defines global
class SceneProperties(bpy.types.PropertyGroup):
    props = bpy.props

    hdri_image: props.PointerProperty(type = bpy.types.Image, name = "HDRI Image", description = "Sky in engine will use this hdri image for lighting calculations")

    save_filepath: props.StringProperty(name = "Save filepath", default = "", description = "Filepath where scene file should be saved")

#Scene export button
class SMOOTHIE_OT_SceneExportButton(bpy.types.Operator, ExportHelper):
    bl_idname = "smoothie.scene_export_button"
    bl_label = "Export Scene"
    filter_glob: bpy.props.StringProperty(default = '*.sscene', options = {'HIDDEN'})
    
    filename_ext = '.sscene' 
    
    def execute(self, context):
        bpy.context.scene.SmoothieExportSettings.save_filepath = self.filepath
        writeSceneFile()
        return {'FINISHED'}

def vector4ToString(vector) -> str:
    x = vector[0]
    y = vector[1]
    z = vector[2]
    w = vector[3]
    return f"{x:.6f} {y:.6f} {z:.6f} {w:.6f}"

def vector3ToString(vector) -> str:
    x = vector[0]
    y = vector[1]
    z = vector[2]
    return f"{x:.6f} {y:.6f} {z:.6f}"

def getLocalMatrix(model_node: ET.Element, object) -> ET.Element:
    
    #Change blender coordinate system to one that smoothie uses
    new_matrix = mathutils.Matrix(((1, 0, 0, 0), (0, 0, 1, 0), (0, -1, 0, 0), (0, 0, 0, 1))) @ object.matrix_world
    
    row0 = ET.SubElement(model_node, "row0")
    row0.text = vector4ToString(new_matrix[0])
    
    row1 = ET.SubElement(model_node, "row1")
    row1.text = vector4ToString(new_matrix[1])
    
    row2 = ET.SubElement(model_node, "row2")
    row2.text = vector4ToString(new_matrix[2])
    return model_node

def getModelFilePath(sceneFilePath: str, objectName: str):

    base_file_path = sceneFilePath.split("\\")
    base_file_path.pop(-1)
    base_file_path.append(objectName + ".smodel")
    filepath = "\\".join(base_file_path)
    return filepath
        
def writeSceneFile(): 
    
    export_settings = bpy.context.scene.SmoothieExportSettings
    scene_objects = bpy.context.scene.objects

    scene_meshes = []
    for object in scene_objects:
        if object.type == "MESH":
           if object.SmoothieExportSettings.ignore_at_all == False:
                scene_meshes.append(object)

    
    root = ET.Element("root")

    #Hdri image
    skybox = ET.SubElement(root, "skybox")
    hdri_image_path = ""
    try:
        hdri_image_path = "resources\\" + export_settings.hdri_image.filepath.split("resources\\")[1]
    except IndexError:
        print("Warning! HDRI image file is not inside resouces folder!")
        hdri_image_path = export_settings.hdri_image.filepath
    except AttributeError:
        print("ERROR! Scene file must have HDRI image seelcted for export!")
        return None
    skybox.text = hdri_image_path


    #Scene models
    models = ET.SubElement(root, "models")
    
    for model in scene_meshes:

        model.select_set(True)
        bpy.context.view_layer.objects.active = model

        
        model_node = ET.SubElement(models, "model")

        model_position = ET.SubElement(model_node, "position")
        model_position.text = vector3ToString(model.location)

        model_scale = ET.SubElement(model_node, "scale")
        model_scale.text = vector3ToString(model.scale)

        model_rotation = ET.SubElement(model_node, "rotation")
        model_rotation.text = vector3ToString((model.rotation_euler[0] * (180/math.pi), model.rotation_euler[1] * (180/math.pi), model.rotation_euler[2] * (180/math.pi)))

        #Write matrix
        # model_matrix = ET.SubElement(model_node, "matrix")
        # getLocalMatrix(model_matrix, model)
        
        #Write path to model file
        model_file = ET.SubElement(model_node, "file")
        model_filepath = ""

        #TODO: Make this better
        if(model.SmoothieExportSettings.save_filepath == ""):
            print("Warning! Model \"" + model.name + "\" is not saved anywhere! It will be saved in the same folder as scene file.")
            model_filepath = getModelFilePath(export_settings.save_filepath, model.name)
            model.SmoothieExportSettings.save_filepath = model_filepath
            model_script.WriteModelFile()
        else:
            model_filepath = model.SmoothieExportSettings.save_filepath
            if model.SmoothieExportSettings.only_update_position == False:
                model_script.WriteModelFile()
        
        try:
            model_filepath = "resources\\" + model.SmoothieExportSettings.save_filepath.split("resources\\")[1]
            model_file.text = model_filepath
        except IndexError:
            print("Warning! Model \"" + model.name + "\" is not saved inside resources folder! Full path will be used!")
            model_file.text = model.SmoothieExportSettings.save_filepath

        model.select_set(False)




    ET.ElementTree(root).write(export_settings.save_filepath)
    return None

def registerClasses():
    bpy.utils.register_class(SceneProperties)
    bpy.utils.register_class(SMOOTHIE_OT_SceneExportButton)
    bpy.types.Scene.SmoothieExportSettings = bpy.props.PointerProperty(type=SceneProperties)

    return None

def unregisterClasses():
    bpy.utils.unregister_class(SceneProperties)
    bpy.utils.unregister_class(SMOOTHIE_OT_SceneExportButton)
    del bpy.types.Scene.SmoothieExportSettings

    return None