#include "Descriptor.h"
#include "Image.h"
#include "Core/SmoothieCore.h"
#include <iostream>

int Smoothie::DescriptorSetData::create(const ShaderFile& shaderFile, unsigned int targetSetIndex, const std::vector<ShaderFile::SearchKey>& searchKeys)
{
	std::unordered_map<int, ShaderObjectReflection> __per_binding_reflections;
	std::unordered_map<int, std::vector<ShaderObjectReflection>> __per_binding_buffer_reflections;
	const auto& __shaders = shaderFile.shaders;
	for (const auto& __key: searchKeys)
	{
		if (__shaders.find(__key) == __shaders.end()) continue;//Skip unintresting shaders
		
		//Blocked variables like textures, buffers, etc.
		for (const auto& __reflection : __shaders.at(__key).reflections)
		{
			if (__reflection.set != targetSetIndex) continue; //Skip unintresting descriptor sets

			if (__per_binding_reflections.find(__reflection.binding) != __per_binding_reflections.end())
			{	
				auto& __existing_reflection = __per_binding_reflections.at(__reflection.binding);
				if (__existing_reflection.type != __reflection.type)
				{
					std::cout << "Multiple shaders share same descriptor set and binding but different types: " << __reflection.type << " and " << __existing_reflection.type << std::endl;
					return 1;
				}

				auto& __stage_bits = __per_binding_reflections[__reflection.binding].stages;
				__stage_bits = static_cast<VkShaderStageFlagBits>(__stage_bits | __reflection.stages);
			}
			else
			{
				__per_binding_reflections[__reflection.binding] = __reflection;
			}
		}
	
		//Buffer variables
		for (const auto& __reflection : __shaders.at(__key).reflections)
		{
			if (__reflection.owning_set != targetSetIndex) continue;
			
			__per_binding_buffer_reflections[__reflection.owning_set_binding].push_back(__reflection);
		}

		//Samplers and combined image samplers.
		for (const auto& __reflection : __shaders.at(__key).reflections)
		{
			if (__reflection.set != targetSetIndex) continue;

			const bool __is_sampler =
				(__reflection.type == MODEL_DESCRIPTOR_SAMPLER_1D) ||
				(__reflection.type == MODEL_DESCRIPTOR_SAMPLER_2D) ||
				(__reflection.type == MODEL_DESCRIPTOR_SAMPLER_3D) ||
				(__reflection.type == MODEL_DESCRIPTOR_SAMPLER_CUBE);

			const bool __is_image = 
				(__reflection.type == MODEL_DESCRIPTOR_IMAGE_1D) ||
				(__reflection.type == MODEL_DESCRIPTOR_IMAGE_2D) ||
				(__reflection.type == MODEL_DESCRIPTOR_IMAGE_3D) ||
				(__reflection.type == MODEL_DESCRIPTOR_IMAGE_CUBE);
			
			if (__is_sampler)
			{

				if (m_Samplers.find(__reflection.name) != m_Samplers.end()) 
				{
					const auto& __sampler = m_Samplers.at(__reflection.name);
					if ((__sampler.binding != __reflection.binding) || (__sampler.type != __reflection.type))
					{
						std::cout << "Sampler " << __reflection.name << " has different binding or type between different shaders!" << std::endl;
						return 1;
					}
				}
				else
				{
					auto& __sampler = m_Samplers[__reflection.name];
					__sampler.binding = __reflection.binding;
					__sampler.type = __reflection.type;
				}
			}
		}

	}
	
	//********************** Descriptor pool sizes **********************//
	std::unordered_map<VkDescriptorType, int> __pool_sizes_per_type;
	for (const auto& [__binding, __reflection]: __per_binding_reflections)
	{
		int __num_of_elements = (__reflection.index <= 0) ? 1 : __reflection.index;
		VkDescriptorType __descriptor_type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
		switch (__reflection.type)
		{
		case MODEL_DESCRIPTOR_STORAGE_BLOCK:
			__descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			break;
		case MODEL_DESCRIPTOR_UNIFORM_BLOCK:
			__descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			break;
		case MODEL_DESCRIPTOR_SAMPLER_2D:
			__descriptor_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			break;

		default:
			std::cout << "Data type: " << static_cast<int>(__reflection.type) << " is not supported yet!" << std::endl; return 1;
			break;
		}
		__pool_sizes_per_type[__descriptor_type] += __num_of_elements;

		VkDescriptorSetLayoutBinding __binding = {};
		__binding.binding = __reflection.binding;
		__binding.descriptorCount = __num_of_elements;
		__binding.descriptorType = __descriptor_type;
		__binding.stageFlags = __reflection.stages;
		m_LayoutBindings.push_back(__binding);
	}

	for (const auto& [__type, __count]: __pool_sizes_per_type)
	{
		VkDescriptorPoolSize __pool_size = {};
		__pool_size.type = __type;
		__pool_size.descriptorCount = __count;
		m_PoolSizes.push_back(__pool_size);
	}

	//********************** Buffer variables ***************************//
	for (const auto& [__index, __variables]: __per_binding_buffer_reflections)
	{
		if (__per_binding_reflections.find(__index) == __per_binding_reflections.end())
		{
			std::cout << "Index of an owning buffer does not match buffers in this descriptor set bindings! Strange error!" << std::endl;
			return 1;
		}
		const auto& __buffer_reflection = __per_binding_reflections.at(__index);
		auto& __current_buffer = m_Buffers[__buffer_reflection.name];
		__current_buffer.binding = __buffer_reflection.binding;

		switch (__buffer_reflection.type)
		{
		case MODEL_DESCRIPTOR_STORAGE_BLOCK:
			__current_buffer.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			break;

		case MODEL_DESCRIPTOR_UNIFORM_BLOCK:
			__current_buffer.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			break;

		default:
			std::cout << "Unsuported type for a buffer: " << static_cast<int>(__buffer_reflection.type) << std::endl;
			break;
		}

		//Copy buffer data to reflections.
		__current_buffer.size = __buffer_reflection.size;
		for (size_t i = 0; i < __variables.size(); i++)
		{
			const auto& __variable_reflection = __variables[i];
			if (__current_buffer.variables.find(__variable_reflection.name) != __current_buffer.variables.end())
			{
				const auto& __loaded_val = __current_buffer.variables.at(__variable_reflection.name);
				if (__loaded_val.offset != __variable_reflection.offset || 
					__loaded_val.type != __variable_reflection.type || 
					__loaded_val.size != __variable_reflection.size)
				{
					return 1;
				}
			}

			auto& __buffer = __current_buffer.variables[__variable_reflection.name];
			__buffer.size = __variable_reflection.size;
			__buffer.type = __variable_reflection.type;
			__buffer.offset = __variable_reflection.offset;
		}

	}


	return 0;
}
