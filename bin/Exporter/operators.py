import bpy #type: ignore
from bpy_extras.io_utils import ExportHelper, ImportHelper #type: ignore
import threading

#Helper class to send data to exporter
class ExporterData:
    model_save_filepath: str
    model_name: str
    model_data_str: str
    
    selected_shader: str
    geometry_save_filepath: str

    number_of_vertices: int
    vertex_attributes: list
    saved_textures: list

    bbMin: str
    bbMax: str


class AttributeData:
    attribute_name: str
    attribute_type: str
    attribute_data: list


def otherThreadRender(data):
    import SmoothieExporter
    result = str()
    result = SmoothieExporter.run_exporter(data)
    data.model_data_str = result
    return None

def vertexAttributesToList(collectionData, type) -> list:
    if (type == "FLOAT") or (type == "INT") or (type == "BOOL") or (type == "INT8"):
        return_data = [0] * len(collectionData)
        collectionData.foreach_get("value", return_data)
        return return_data 
    
    if (type == "FLOAT2"):
        return_data = [0] * len(collectionData) * 2
        collectionData.foreach_get("vector", return_data)
        return return_data 

    if (type == "INT32_2D"):
        return_data = [0] * len(collectionData) * 2
        collectionData.foreach_get("value", return_data)
        return return_data 
    
    if (type == "FLOAT_VECTOR"):
        return_data = [0] * len(collectionData) * 3
        collectionData.foreach_get("vector", return_data)
        return return_data 
    
    if (type == "FLOAT_COLOR") or (type == "BYTE_COLOR"):
        return_data = [0] * len(collectionData) * 4
        collectionData.foreach_get("color", return_data)
        return return_data 
    
    if (type == "QUATERNION"):
        return_data = [0] * len(collectionData) * 4
        collectionData.foreach_get("value", return_data)
        return return_data 

    return []        

        
class ModelExportButton(bpy.types.Operator, ExportHelper):
    bl_idname = "smoothie.export_model_data"
    bl_label = "Export Smoothie Model"

    filter_glob: bpy.props.StringProperty(default = '*.smodel', options = {'HIDDEN'}) #type: ignore
    filename_ext = ".smodel"

    def execute(self, context):
        obj = context.object
        
        data = ExporterData()
        data.model_save_filepath = self.filepath
        data.model_name = obj.name
        data.model_data_str = obj.smoothie_exporter_data.model_data
        data.selected_shader = obj.smoothie_exporter_data.selected_shader
        data.geometry_save_filepath = obj.smoothie_exporter_data.selected_geometry_file
        data.number_of_vertices = len(obj.data.vertices)

        data.bbMin = str(round(obj.bound_box[4][0], 4)) + " " + str(round(obj.bound_box[4][1], 4)) + " " + str(round(obj.bound_box[4][2], 4))
        data.bbMax = str(round(obj.bound_box[2][0], 4)) + " " + str(round(obj.bound_box[2][1], 4)) + " " + str(round(obj.bound_box[2][2], 4))

        #Vertex attributes
        vertex_attribute_data = []
        for attribute in obj.data.attributes:
            if (attribute.domain != 'POINT') or (attribute.is_internal == True): continue
            attribute_data = AttributeData()

            attribute_data.attribute_name = attribute.name
            attribute_data.attribute_type = attribute.data_type
            attribute_data.attribute_data = vertexAttributesToList(attribute.data, attribute.data_type)
            vertex_attribute_data.append(attribute_data)
        data.vertex_attributes = vertex_attribute_data

        #Textures
        texture_data = set()
        for image in bpy.data.images:
            if image.filepath:
                texture_data.add(bpy.path.abspath(image.filepath))
        data.saved_textures = list(texture_data)
        
        #this MUST be done on different thread because it creates new OpenGL window and new context
        #When that context gets destroyed blender crashes because it looses its original context and there is no way to return it
        #this way every thread has its own context 
        try:
            thread = threading.Thread(target=otherThreadRender, args=(data,))
            thread.start()
            thread.join()

        except Exception:
            None
        
        obj.smoothie_exporter_data.model_data = data.model_data_str
        return {'FINISHED'}


class SelectShaderButton(bpy.types.Operator, ImportHelper):
    bl_idname = "smoothie.load_shader_data"
    bl_label = "Select Smoothie Shader"

    filter_glob: bpy.props.StringProperty(default = '*.sshader', options = {'HIDDEN'}) #type: ignore
    filename_ext = ".sshader"

    def execute(self, context):
        
        context.object.smoothie_exporter_data.selected_shader = self.filepath

        return {'FINISHED'}

class SelectGeometryButton(bpy.types.Operator, ExportHelper):
    bl_idname = "smoothie.slect_geometry_file"
    bl_label = "Select geometry file"

    filter_glob: bpy.props.StringProperty(default = '*.sgeometry', options = {'HIDDEN'}) #type: ignore
    filename_ext = ".sgeometry"

    def execute(self, context):
        
        context.object.smoothie_exporter_data.selected_geometry_file = self.filepath

        return {'FINISHED'}
   
def register_classes():
    bpy.utils.register_class(ModelExportButton)
    bpy.utils.register_class(SelectShaderButton)
    bpy.utils.register_class(SelectGeometryButton)

def unregister_classes():
    bpy.utils.unregister_class(ModelExportButton)
    bpy.utils.unregister_class(SelectShaderButton)
    bpy.utils.unregister_class(SelectGeometryButton)