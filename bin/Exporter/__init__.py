import bpy #type: ignore
import operators

#Main UI class
class SmoothieExporter(bpy.types.Panel):    
    bl_label = "Model Export"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "Smoothie Export"
    bl_idname = "SMOOTHIE_PT_SmoothieExporter"
    
    def draw(self, context):
        obj = context.object
        layout = self.layout
        
        if(obj == None):
            layout.enabled = False
            return
        
        layout.label(text = "Selected data for exporting:")
        box = layout.box()
        box.prop(obj.smoothie_exporter_data, "selected_shader", text = "shader")
        box.prop(obj.smoothie_exporter_data, "selected_geometry_file", text = "geometry")
        box.enabled = False

        layout.operator("smoothie.load_shader_data")
        layout.operator("smoothie.slect_geometry_file")
        layout.operator("smoothie.export_model_data")

class SmoothieExporterData(bpy.types.PropertyGroup):
    model_data: bpy.props.StringProperty(name = "model_data") #type: ignore
    selected_shader: bpy.props.StringProperty(name = "Selected Shader") #type: ignore
    selected_geometry_file: bpy.props.StringProperty(name = "Selected geometry file") #type: ignore

def register(): 
    operators.register_classes()
    bpy.utils.register_class(SmoothieExporter)
    bpy.utils.register_class(SmoothieExporterData)

    bpy.types.Object.smoothie_exporter_data = bpy.props.PointerProperty(type = SmoothieExporterData)

def unregister():
    operators.unregister_classes()
    bpy.utils.unregister_class(SmoothieExporter)
    bpy.utils.unregister_class(SmoothieExporterData)
    del bpy.types.Object.smoothie_exporter_data



if __name__ == "__main__":
    register()