#include "Buffer.h"
#include <vector>
#include <memory>
#include <cstring>
#include "Core/SmoothieCore.h"
#include "Core/Multithreading.h"
#include <iostream>

int Smoothie::DefaultBuffer::create()
{
	VkBufferCreateInfo __BufferInfo{};
	__BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	__BufferInfo.size = 1024;
	__BufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VmaAllocationCreateInfo __allocation_BufferInfo{};
	__allocation_BufferInfo.usage = VMA_MEMORY_USAGE_AUTO;
	__allocation_BufferInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;


	if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &__BufferInfo, &__allocation_BufferInfo, &buffer, &bufferAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create uniform buffer!" << std::endl;
		return 1;
	}

	ImmediateCommandBuffer _commandBuffer;
	if (_commandBuffer.create() != 0)
	{
		std::cout << "Failed to create immediate command buffer!" << std::endl;
		return 1;
	}
	_commandBuffer.begin();
	vkCmdFillBuffer(_commandBuffer.get_CommandBuffer(), buffer, 0, VK_WHOLE_SIZE, 0);
	_commandBuffer.end();
	_commandBuffer.submit();
	_commandBuffer.destroy();

	return 0;
}

void Smoothie::DefaultBuffer::resize_callback()
{
}

void Smoothie::DefaultBuffer::destroy()
{
	vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), buffer, bufferAllocation);
	buffer = nullptr, bufferAllocation = nullptr;
}
