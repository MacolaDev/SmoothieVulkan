import bpy #type: ignore

import scene
import model

#This is here so that I dont have to close blender every time I make a change in other files.
#It shouldnt be here in the final verion of script. There will be more of theese in other scripts too.
import importlib
importlib.reload(model)
importlib.reload(scene)

#Layout class for exporting models
class SmoothieExporter(bpy.types.Panel):    
    bl_label = "Model Export"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "Smoothie Export"
    bl_idname = "SMOOTHIE_PT_SmoothieExporter"
    
    def draw(self, context):
        obj = bpy.context.object
        layout = self.layout
        if(obj == None):
            layout.enabled = False
            return

        export = obj.SmoothieExportSettings
        

        #Scene export settings
        layout.label(text ="Scene export settings")

        layout.prop(export, "ignore_at_all")
        if export.ignore_at_all == False:
            layout.prop(export, "only_update_position")
        
        #Geometry file setting
        layout.label(text = "Mesh export settings")
        layout.prop(export, "export_mesh")
        if export.export_mesh == False:
            layout.prop(export, "export_mesh_file")
            layout.operator("smoothie.import_sgeometry_file_button", text = "Select .geometry file", icon = "SCENE_DATA")

        #Shader properties
        layout.label(text = "Shader properties")
        layout.prop(export, "double_sided")
        layout.prop(export, "useAlpha")

        layout.prop(export, "selected_shader")
        obj.SmoothieShaderSettings.draw(layout)
        
        layout.label(text =  "Export model file:")
        split = layout.split()
        split.prop(export, "save_filepath")
        split.enabled = False
        layout.operator("smoothie.model_export_button", text = "Export Model", icon = "CUBE")

#Layout class for exporting scenes
class SmoothieSceneExporter(bpy.types.Panel):    
    bl_label = "Scene Export (In development!)"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "Smoothie Export"
    bl_idname = "SMOOTHIE_PT_SmoothieSceneExporter"

    
    def draw(self, context):
        layout = self.layout
        export = bpy.context.scene.SmoothieExportSettings
        
        layout.label(text = "Sky settings")
        layout.prop(export, "hdri_image")
        
        layout.label(text =  "Export scene file:")

        #Makes save filepath variable read-only
        split = layout.split()
        split.prop(export, "save_filepath")
        split.enabled = False

        layout.operator("smoothie.scene_export_button", text = "Export Scene", icon = "SCENE_DATA")

def register(): 
    
    model.registerClasses()
    scene.registerClasses()

    bpy.utils.register_class(SmoothieExporter)
    bpy.utils.register_class(SmoothieSceneExporter)

def unregister():
    model.unregisterClasses()
    scene.unregisterClasses()

    bpy.utils.unregister_class(SmoothieExporter)
    bpy.utils.unregister_class(SmoothieSceneExporter)


if __name__ == "__main__":
    register()