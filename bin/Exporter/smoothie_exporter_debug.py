import SmoothieExporter

file = "D:\SoftwareDev_Projects\SmoothieVulkan\Demo\shaders\pbr\pbr.sshader"


class ToAnalyze:
    shader_filepath: str
    vertex_shader_input = []
    uniform_data_input = []

data = ToAnalyze()
data.shader_filepath = file
SmoothieExporter.analyze_shader_file(data)

print(data.vertex_shader_input, data.uniform_data_input)