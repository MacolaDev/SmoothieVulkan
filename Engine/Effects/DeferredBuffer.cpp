#include "DeferredBuffer.h"
#include "Core/SmoothieCore.h"
#include <cstring>
int Smoothie::DeferredRendering::UniformBuffer_Base::create()
{
	return 0;
}

int Smoothie::DeferredRendering::UniformBuffer_Static::create(
	const std::unordered_map<std::string, Smoothie::ShaderObjectReflection>& _reflections,
	const std::vector<XML::Element>& _properties,
	const Reserved_Values& _reserved_values)
{
	if (_reflections.find("ModelData") == _reflections.end())
	{
		std::cout << "Failed to find the ModelData descriptor set!" << std::endl;
		return 1;
	}

	const auto& __ModelData_UniformBuffer = _reflections.at("ModelData");
	if (__ModelData_UniformBuffer.size < 0)
	{
		std::cout << "Uniform buffer of size " << __ModelData_UniformBuffer.size << " is invalid!" << std::endl;
		return 1;
	}

	if (__ModelData_UniformBuffer.binding < 0)
	{
		std::cout << "Invalid binding number: " << __ModelData_UniformBuffer.size << std::endl;
		return 1;
	}


	//*************************** BUFFER CREATION **************************//
	VkBufferCreateInfo __buffer_create_info{};
	__buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	__buffer_create_info.size = __ModelData_UniformBuffer.size;
	__buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	__buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	__buffer_create_info.queueFamilyIndexCount = 0;

	VmaAllocationCreateInfo __buffer_allocation_create_info{};
	__buffer_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
	__buffer_allocation_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &__buffer_create_info, &__buffer_allocation_create_info,
		&buffer, &bufferAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create uniform buffer!" << std::endl;
		return 1;
	}



	std::vector<char> __buffer_data(__ModelData_UniformBuffer.size);

	//*************************** modelMatrix **************************//
	if (_reflections.find("modelMatrix") != _reflections.end())
	{
		const auto& _model_matrix_reflection = _reflections.at("modelMatrix");
		if ((_model_matrix_reflection.offset < 0) || (_model_matrix_reflection.type != MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_MAT4))
		{
			std::cout << "modelMatrix must be of type mat4!" << std::endl;
		}
		else
		{
			std::memcpy(&__buffer_data[_model_matrix_reflection.offset], &_reserved_values.modelMatrix, sizeof(_reserved_values.modelMatrix));
		}
	}

	//*************************** ModelID **************************//
	if (_reflections.find("ModelID") != _reflections.end())
	{
		const auto& _model_matrix_reflection = _reflections.at("ModelID");
		if ((_model_matrix_reflection.offset < 0) || (_model_matrix_reflection.type != MODEL_DESCRIPTOR_DATA_TYPE_UNSIGNED_INT))
		{
			std::cout << "ModelID must be of type uint!" << std::endl;
		}
		else
		{
			std::memcpy(&__buffer_data[_model_matrix_reflection.offset], &_reserved_values.ModelID, sizeof(_reserved_values.ModelID));
		}
	}

	//************************** Custom Data *************************//
	for (const auto& _property : _properties)
	{
		if ((_property.name == "Texture") || (_property.name == "Texture1D") || (_property.name == "Texture2D") || (_property.name == "Texture3D")) continue;

		
		
	}

	if(vmaCopyMemoryToAllocation(SmoothieCore::getVulkanMemoryAllocator(), __buffer_data.data(), bufferAllocation, 0, __ModelData_UniformBuffer.size) != VK_SUCCESS)
	{
		std::cout << "Failed to copy the memory from bufer data to buffer on the gpu!" << std::endl;
		return 1;
	}
    return 0;
}

void Smoothie::DeferredRendering::UniformBuffer_Static::destroy()
{
	vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), buffer, bufferAllocation);
	buffer = nullptr, bufferAllocation = nullptr;
}

void Smoothie::DeferredRendering::UniformBuffer_Static::resize_callback()
{
}
