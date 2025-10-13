import bpy #type: ignore
from bpy_extras.io_utils import ExportHelper, ImportHelper #type: ignore

class Shader(bpy.types.PropertyGroup):
    #Shader name
    shader: bpy.props.StringProperty() #type: ignore

loaded_shaders_names = []
loaded_shaders_vertex_data_pointers = {}
loaded_shaders_uniform_data_pointers = {}

def getLoadedShader(self, context):
    return loaded_shaders_names

class SmoothieModelData(bpy.types.PropertyGroup):
    loaded_shaders: bpy.props.EnumProperty(
        items = getLoadedShader, 
        name = "Loaded Shaders") #type: ignore

class ShaderAnalyzer:
    shader_filepath: str
    vertex_shader_input = []
    uniform_data_input = []

def createVertexInputClass(name, dataList):
    
    for data in dataList:
        data[2]


    return type(name + "_TEST_SHADER", (bpy.types.PropertyGroup, ))

class SmoothieShaderOpenAddOperator(bpy.types.Operator, ImportHelper):
    bl_idname = "smoothie_model_data.import_sshader_file"
    bl_label = "Add shader file"

    filter_glob: bpy.props.StringProperty(default = '*.sshader', options = {'HIDDEN'}) #type: ignore

    def execute(self, context):

        for data in loaded_shaders_names:
            if data[0] == self.filepath:
                return {'FINISHED'}
        
        identifier = self.filepath.split("\\")
        identifier = identifier[len(identifier) - 1]
        loaded_shaders_names.append((identifier, identifier, ""))
        

        import SmoothieExporter
        shaderDataToAnalyze = ShaderAnalyzer()
        shaderDataToAnalyze.shader_filepath = self.filepath
        shaderDataToAnalyze.uniform_data_input = []
        shaderDataToAnalyze.vertex_shader_input = []
        SmoothieExporter.analyze_shader_file(shaderDataToAnalyze) #analyzes shader data and fills empty lists with data


        loaded_shaders_vertex_data_pointers[self.filepath] = createVertexInputClass(identifier, shaderDataToAnalyze.vertex_shader_input)
        loaded_shaders_uniform_data_pointers[self.filepath] = object

        print(loaded_shaders_vertex_data_pointers)
        return {'FINISHED'}

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
        
        layout.prop(obj.smoothie_model_data, "loaded_shaders")
        layout.operator("smoothie_model_data.import_sshader_file")
        
        
def register(): 
    bpy.utils.register_class(SmoothieExporter)
    bpy.utils.register_class(Shader)
    bpy.utils.register_class(SmoothieModelData)

    bpy.types.Object.smoothie_model_data = bpy.props.PointerProperty(type = SmoothieModelData)
    bpy.utils.register_class(SmoothieShaderOpenAddOperator)


def unregister():

    bpy.utils.unregister_class(SmoothieShaderOpenAddOperator)
    del bpy.types.Object.smoothie_model_data

    bpy.utils.unregister_class(SmoothieModelData)
    bpy.utils.unregister_class(Shader)
    bpy.utils.unregister_class(SmoothieExporter)


if __name__ == "__main__":
    register()