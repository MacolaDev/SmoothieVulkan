#include "CameraDescriptor.h"
#include "Core/SmoothieCore.h"
#include <iostream>

void Smoothie::CameraDescriptorSet::resize_callback(){}

int Smoothie::CameraDescriptorSet::create()
{

	if (buffer.create() != 0)
	{
		std::cout << "Failed to create buffer!" << std::endl;
		return 1;
	}

	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;
	
	if(vkCreateDescriptorPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
	{
		std::cout << "Failed to create camera descriptor pool!" << std::endl;
		return 1;
	}
	

	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = 0;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &layoutBinding;
	if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create camera descriptor set layout!" << std::endl;
		return 1;
	}

	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = m_DescriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &m_DescriptorSetLayout;
	if (vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &m_DescriptorSet) != VK_SUCCESS)
	{
		std::cout << "Failed to allocate camera descriptor set!" << std::endl;
		return 1;
	}

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer.getBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = m_DescriptorSet;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr;

	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
	return 0;
}

void Smoothie::CameraDescriptorSet::update_camera_data(const CameraUniformBufferData& data)
{
	vmaCopyMemoryToAllocation(SmoothieCore::getVulkanMemoryAllocator(), &data, buffer.getAllocation(), 0, sizeof(CameraUniformBufferData));
}

void Smoothie::CameraDescriptorSet::destroy()
{
	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), m_DescriptorSetLayout, nullptr);
	m_DescriptorSetLayout = nullptr;

	vkDestroyDescriptorPool(SmoothieCore::getDevice(), m_DescriptorPool, nullptr);
	m_DescriptorPool = nullptr, m_DescriptorSet = nullptr;

	buffer.destroy();
}

int Smoothie::CameraDescriptorBuffer::create()
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = sizeof(CameraUniformBufferData);
	bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &bufferCreateInfo, &allocInfo, &buffer, &bufferAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create camera buffer!" << std::endl;
		return 1;
	}

	return 0;
}

void Smoothie::CameraDescriptorBuffer::resize_callback(){}

void Smoothie::CameraDescriptorBuffer::destroy()
{
	vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), buffer, bufferAllocation);
	buffer = nullptr, bufferAllocation = nullptr;
}
