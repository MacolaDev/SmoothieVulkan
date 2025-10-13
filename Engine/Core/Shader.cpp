#include "Shader.h"
#include <iostream>
#include <fstream>
#include "SmoothieCore.h"
#include <filesystem>
#include <mutex>
#include "ResourceManager/ShaderFile.h"
#include <algorithm>

using namespace Smoothie;

int Smoothie::get_VkFormat_from_ModelDescriptorDataType(ModelDescriptorDataType type, VkFormat& result, unsigned int& format_size)
{
	switch (type)
	{
	case MODEL_DESCRIPTOR_DATA_TYPE_BYTE:
		result = VK_FORMAT_R8_SINT;
		format_size = 1;
		break;
	case MODEL_DESCRIPTOR_DATA_TYPE_UNSIGNED_BYTE:
		result = VK_FORMAT_R8_UINT;
		format_size = 1;
		break;
	case MODEL_DESCRIPTOR_DATA_TYPE_SHORT:
		result = VK_FORMAT_R16_SINT;
		format_size = 2;
		break;
	case MODEL_DESCRIPTOR_DATA_TYPE_UNSIGNED_SHORT:
		result = VK_FORMAT_R16_UINT;
		format_size = 2;
		break;


	case MODEL_DESCRIPTOR_DATA_TYPE_FLOAT:
		result = VK_FORMAT_R32_SFLOAT;
		format_size = 4;
		break;

	case MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_VEC2:
		result = VK_FORMAT_R32G32_SFLOAT;
		format_size = 8;
		break;
	case MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_VEC3:
		result = VK_FORMAT_R32G32B32_SFLOAT;
		format_size = 12;
		break;

	case MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_VEC4:
		result = VK_FORMAT_R32G32B32A32_SFLOAT;
		format_size = 16;
		break;


	case MODEL_DESCRIPTOR_DATA_TYPE_DOUBLE:
		result = VK_FORMAT_R64_SFLOAT;
		format_size = 8;
		break;
	case MODEL_DESCRIPTOR_DATA_TYPE_DOUBLE_VEC2:
		result = VK_FORMAT_R64G64_SFLOAT;
		format_size = 16;
		break;
	case MODEL_DESCRIPTOR_DATA_TYPE_DOUBLE_VEC3:
		result = VK_FORMAT_R64G64B64_SFLOAT;
		format_size = 24;
		break;
	case MODEL_DESCRIPTOR_DATA_TYPE_DOUBLE_VEC4:
		result = VK_FORMAT_R64G64B64A64_SFLOAT;
		format_size = 32;
		break;


	case MODEL_DESCRIPTOR_DATA_TYPE_INT:
		result = VK_FORMAT_R32_SINT;
		format_size = 4;
		break;
	case MODEL_DESCRIPTOR_DATA_TYPE_INT_VEC2:
		result = VK_FORMAT_R32G32_SINT;
		format_size = 8;
		break;
	case MODEL_DESCRIPTOR_DATA_TYPE_INT_VEC3:
		result = VK_FORMAT_R32G32B32_SINT;
		format_size = 12;
		break;
	case MODEL_DESCRIPTOR_DATA_TYPE_INT_VEC4:
		result = VK_FORMAT_R32G32B32A32_SINT;
		format_size = 16;
		break;


	case MODEL_DESCRIPTOR_DATA_TYPE_UNSIGNED_INT:
		result = VK_FORMAT_R32_UINT;
		format_size = 4;
		break;
	case MODEL_DESCRIPTOR_DATA_TYPE_UNSIGNED_INT_VEC2:
		result = VK_FORMAT_R32G32_UINT;
		format_size = 8;
		break;
	case MODEL_DESCRIPTOR_DATA_TYPE_UNSIGNED_INT_VEC3:
		result = VK_FORMAT_R32G32B32_UINT;
		format_size = 12;
		break;
	case MODEL_DESCRIPTOR_DATA_TYPE_UNSIGNED_INT_VEC4:
		result = VK_FORMAT_R32G32B32A32_UINT;
		format_size = 16;
		break;

	default:
		return 1;
	}

	return 0;
}

int Smoothie::Add_system_shader(const std::string& shaderFile, std::unordered_map<std::string, VkShaderModule>& modules)
{
	if (!std::filesystem::exists(shaderFile))
	{
		std::cout << "No file named: " << shaderFile << std::endl;
		return 1;
	}

	size_t file_size = std::filesystem::file_size(shaderFile);
	std::vector<char> binary_data(file_size);
	auto file = std::ifstream(shaderFile, std::ios_base::binary);
	file.read(binary_data.data(), file_size);
	file.close();

	_ShaderFile_file_data file_data;
	if (file_data.get_object_from_buffer(binary_data) != 0)
	{
		std::cout << "Failed to parse file: " << shaderFile << std::endl;
		return 1;
	}

	for (const auto& shader : file_data.shaders)
	{
		if (modules.find(shader.shader_name) != modules.end())
		{
			std::cout << "System shader with name: " << shader.shader_name << " already exists and its loaded into memory!" << std::endl;
			return 1;
		}
		
		
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shader.SPIRV_data.size() * 4;
		createInfo.pCode = shader.SPIRV_data.data();
		VkShaderModule __shader__module = nullptr;
		if (vkCreateShaderModule(SmoothieCore::getDevice(), &createInfo, nullptr, &__shader__module) != VK_SUCCESS)
		{
			std::cout << "Failed to create VkShaderModule from provided SPIR-V data!" << std::endl;
			return 1;
		}

		modules[shader.shader_name] = __shader__module;

	}

	return 0;
}

static int getShaderModuleFromFile(const _ShaderModule_file_data& file_data, ShaderModule& shader_module)
{
	//Create shader module
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = file_data.SPIRV_data.size() * 4;
	createInfo.pCode = file_data.SPIRV_data.data();
	if (vkCreateShaderModule(SmoothieCore::getDevice(), &createInfo, nullptr, &shader_module.shader_module) != VK_SUCCESS)
	{
		std::cout << "Failed to create VkShaderModule from provided SPIR-V data!" << std::endl;
		return 1;
	}

	//Reflections
	for (size_t i = 0; i < file_data.reflections_count; i++)
	{
		const auto& file_reflection = file_data.reflections[i];

		ShaderObjectReflection reflection;
		reflection.offset = file_reflection.offset;
		reflection.type = file_reflection.data_type;
		reflection.size = file_reflection.size;
		reflection.index = file_reflection.index;
		reflection.counterIndex = file_reflection.counterIndex;
		reflection.numMembers = file_reflection.numMembers;
		reflection.arrayStride = file_reflection.arrayStride;
		reflection.topLevelArraySize = file_reflection.topLevelArraySize;
		reflection.topLevelArrayStride = file_reflection.topLevelArrayStride;
		reflection.stages = static_cast<VkShaderStageFlagBits>(file_reflection.stages);
		
		reflection.location = file_reflection.location;
		reflection.binding = file_reflection.binding;
		reflection.set = file_reflection.set;

		reflection.owning_set = file_reflection.owning_set;
		reflection.owning_set_binding = file_reflection.owning_set_binding;

		reflection.name = file_reflection.name;

		using RFT = _Reflection_type_file_data;
		if (file_reflection.reflection_type == RFT::PIPE_INPUT)
		{
			shader_module.pipe_inputs.push_back(reflection);
		}
		else if (file_reflection.reflection_type == RFT::PIPE_OUTPUT)
		{
			shader_module.pipe_outputs.push_back(reflection);
		}
		else if (
			(file_reflection.reflection_type == RFT::UNIFORM_BLOCK) || 
			(file_reflection.reflection_type == RFT::UNIFORM_VARIABLE) ||
			(file_reflection.reflection_type == RFT::STORAGE_BLOCK) || 
			(file_reflection.reflection_type == RFT::STORAGE_VARIABLE)
			)
		{
			shader_module.reflections.push_back(reflection);
		}
		else
		{
			return 1;
		}
	}
	return 0;
}

static std::mutex mutex;
int Smoothie::ShaderFile::create(const std::string& shaderFile)
{
	std::lock_guard<std::mutex> lock(mutex);
	this->filepath = shaderFile;

	if (!std::filesystem::exists(filepath))
	{
		std::cout << "No file named: " << filepath << std::endl;
		return 1;
	}

	size_t file_size = std::filesystem::file_size(filepath);
	std::vector<char> binary_data(file_size);
	auto file = std::ifstream(filepath, std::ios_base::binary);
	file.read(binary_data.data(), file_size);
	file.close();
	
	_ShaderFile_file_data file_data;
	if (file_data.get_object_from_buffer(binary_data) != 0)
	{
		std::cout << "Failed to parse file: " << filepath << std::endl;
		return 1;
	}

	for (const auto& shader : file_data.shaders)
	{
		if (getShaderModuleFromFile(shader, shaders[{shader.shader_name, static_cast<VkShaderStageFlagBits>(shader.shader_stage)}]) != 0) return 1;
	}
	return 0;
}

void Smoothie::ShaderFile::destroy()
{
	for (auto& [key, shader]: shaders)
	{
		vkDestroyShaderModule(SmoothieCore::getDevice(), shader.shader_module, nullptr);
		shader.shader_module = nullptr;
		shader.shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		shader.pipe_inputs.clear();
		shader.pipe_outputs.clear();
	}
	shaders.clear();
}

int Smoothie::ShaderFile::get_pipeline_data_shaders(const std::string& identifier, VkShaderStageFlagBits stage, VkPipelineShaderStageCreateInfo& createInfo) const
{
	SearchKey key;
	key.name = identifier;
	key.stage = stage;
	if (auto search = shaders.find(key); search != shaders.end())
	{
		const auto& shader = shaders.at(key);
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.pName = "main";
		createInfo.stage = stage;
		createInfo.module = shader.shader_module;
		return 0;
	}

	std::cout << "Failed to get the shader stage!" << identifier << std::endl;
	return 1;
}

int Smoothie::ShaderFile::get_pipeline_data_shaders(const std::string& identifier, VkShaderStageFlagBits stage, std::vector<VkPipelineShaderStageCreateInfo>& createInfos) const
{
	SearchKey key;
	key.name = identifier;
	key.stage = stage;
	if (auto search = shaders.find(key); search != shaders.end())
	{
		const auto& shader = shaders.at(key);
		VkPipelineShaderStageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.pName = "main";
		createInfo.stage = stage;
		createInfo.module = shader.shader_module;
		createInfos.push_back(createInfo);
		return 0;
	}
	return 1;
}

static bool _compare(const ShaderObjectReflection& first, const ShaderObjectReflection& last)
{
	if (first.binding == last.binding) return first.location < last.location;
	return first.binding < first.binding;
}

int Smoothie::ShaderFile::get_pipeline_data_vertex_stage_data(const std::string& identifier, std::vector<VkVertexInputBindingDescription>& vertexShaderBindingDescriptions, std::vector<VkVertexInputAttributeDescription>& vertexShaderAttributeDescriptions) const
{
	SearchKey key;
	key.name = identifier;
	key.stage = VK_SHADER_STAGE_VERTEX_BIT;
	if (auto search = shaders.find(key); search != shaders.end())
	{
		std::vector<ShaderObjectReflection> pipe_inputs = shaders.at(key).pipe_inputs;
		vertexShaderAttributeDescriptions.resize(pipe_inputs.size());

		//Sort firstly by binding then by location
		std::sort(pipe_inputs.begin(), pipe_inputs.end(), _compare);
		int offset = 0;

		//Case where there is only one shader entry
		if (pipe_inputs.size() == 1)
		{
			VkVertexInputAttributeDescription description{};
			description.location = 0;
			description.binding = 0;
			description.offset = 0;
			unsigned int type_size = 0;
			if (get_VkFormat_from_ModelDescriptorDataType(pipe_inputs[0].type, description.format, type_size) != 0)
			{
				std::cout << "Unsoported data format for Vertex Attribute input: " << pipe_inputs[0].type << std::endl;
				return 1;
			}
			vertexShaderAttributeDescriptions[0] = description;
			
			VkVertexInputBindingDescription __description{};
			__description.binding = 0;
			__description.stride = type_size;
			__description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			vertexShaderBindingDescriptions.push_back(__description);
			return 0;
		}

		for (size_t i = 0; i < pipe_inputs.size() - 1; i++)
		{
			const auto& input = pipe_inputs[i];
			const auto& next = pipe_inputs[i + 1];
			unsigned int binding = (input.binding == -1) ? 0 : input.binding;
			
			VkVertexInputAttributeDescription description{};
			description.location = input.location;
			description.binding = binding;
			description.offset = offset;
			unsigned int type_size = 0;
			if (get_VkFormat_from_ModelDescriptorDataType(input.type, description.format, type_size) != 0)
			{
				std::cout << "Unsoported data format for Vertex Attribute input: " << input.type << std::endl;
				return 1;
			}
			offset += type_size;
			
			vertexShaderAttributeDescriptions[i] = description;

			if (input.binding != next.binding) 
			{
				VkVertexInputBindingDescription __description{};
				__description.binding = binding;
				__description.stride = offset;
				__description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				vertexShaderBindingDescriptions.push_back(__description);
				offset = 0;
			}
		}

		//Last one also needs to be included
		const auto& last = pipe_inputs.back();
		VkVertexInputAttributeDescription description{};
		description.location = last.location;
		description.binding = (last.binding == -1) ? 0 : last.binding;
		description.offset = offset;
		unsigned int type_size = 0;
		if (get_VkFormat_from_ModelDescriptorDataType(last.type, description.format, type_size) != 0)
		{
			std::cout << "Unsoported data format for Vertex Attribute input: " << last.type << std::endl;
			return 1;
		}
		offset += type_size;
		vertexShaderAttributeDescriptions[pipe_inputs.size() - 1] = description;
		VkVertexInputBindingDescription __description{};
		__description.binding = (last.binding == -1) ? 0 : last.binding;
		__description.stride = offset;
		__description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexShaderBindingDescriptions.push_back(__description);

		return 0;

	}
	std::cout << "No vertex shader with identifier: " << identifier << std::endl;
	return 1;
}

bool Smoothie::ShaderFile::SearchKey::operator==(const SearchKey& other) const
{
	return (other.name == name) && (other.stage == stage);
}

std::size_t Smoothie::ShaderFile::SearchKeyHash::operator()(const SearchKey& p) const
{
	std::size_t h1 = std::hash<std::string>{}(p.name);
	std::size_t h2 = std::hash<int>{}(p.stage);
	return h1;
}
