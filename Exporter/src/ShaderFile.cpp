#include "ShaderFile.h"
#include <iostream>
#include <filesystem>
#include <fstream>
//
// int SmoothieExporter::parse_shader_file(const std::string& filepath, ShaderFileMap& stages)
// {
//
// 	if (!std::filesystem::exists(filepath))
// 	{
// 		std::cout << "Failed to open the file: " << filepath << ", no such file exists!" << std::endl;
// 		return 1;
// 	}
//
// 	size_t file_size = std::filesystem::file_size(filepath);
// 	std::vector<char> binary_data(file_size);
// 	auto file = std::ifstream(filepath, std::ios_base::binary);
// 	file.read(binary_data.data(), file_size);
// 	file.close();
//
// 	_ShaderFile_file_data data;
// 	if (data.get_object_from_buffer(binary_data) != 0)
// 	{
// 		std::cout << "Failed to parse the shader file!" << std::endl;
// 		return 1;
// 	}
//
// 	for (const auto& stage_data: data.shaders)
// 	{
// 		ShaderKey __key;
// 		__key.name = stage_data.shader_name;
// 		__key.stage = static_cast<VkShaderStageFlagBits>(stage_data.shader_stage);
// 		auto& stage = stages[__key];
//
// 		for (const auto& __reflection: stage_data.reflections)
// 		{
// 			if (__reflection.reflection_type == _Reflection_type_file_data::PIPE_INPUT)
// 			{
// 				ShaderPipeInput __pipe_input;
// 				__pipe_input.location = __reflection.location;
// 				__pipe_input.type = __reflection.data_type;
// 				__pipe_input.name = __reflection.name;
// 				stage.pipeInputs.push_back(__pipe_input);
// 			}
//
// 			if (__reflection.reflection_type == _Reflection_type_file_data::UNIFORM_VARIABLE)
// 			{
// 				ShaderVariable __variable;
// 				__variable.binding = __reflection.binding;
// 				__variable.set = __reflection.set;
// 				__variable.name = __reflection.name;
// 				__variable.type = __variable.type;
// 				stage.pipeVariables.push_back(__variable);
// 			}
// 		}
// 	}
//
// 	return 0;
// }
