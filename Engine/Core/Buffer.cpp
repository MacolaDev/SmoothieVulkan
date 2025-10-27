#include "Buffer.h"
#include <vector>
#include <memory>
#include <cstring>
#include "Core/SmoothieCore.h"
#include "Core/Multithreading.h"

int Smoothie::BufferUniform::create()
{
	//Calculate buffer size
	for (const auto& data : buffer_data) 
	{
		if (data.get() != nullptr) bufferSize += data.get()->typeSize;
	}
	byte_data.resize(bufferSize);

	//Create uniform buffer
	VkBufferCreateInfo modelDataBufferInfo{};
	modelDataBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	modelDataBufferInfo.size = bufferSize;
	modelDataBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	VmaAllocationCreateInfo vmaModelDataBufferInfo{};
	vmaModelDataBufferInfo.usage = VMA_MEMORY_USAGE_AUTO;
	vmaModelDataBufferInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &modelDataBufferInfo, &vmaModelDataBufferInfo, &buffer, &bufferAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create uniform buffer!" << std::endl;
		return 1;
	}
	
	return 0;
}

void Smoothie::BufferUniform::resize_callback()
{
	
	for (const auto& data : buffer_data)
	{
		auto d = data.get();
		if ((d != nullptr) && (d->data != nullptr))
		{
			std::memcpy(&byte_data[d->typeBufferOffset], d->data, d->typeSize);
		}
	}
	vmaCopyMemoryToAllocation(SmoothieCore::getVulkanMemoryAllocator(), byte_data.data(), bufferAllocation, 0, byte_data.size());


}

void Smoothie::BufferUniform::destroy()
{
	vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), buffer, bufferAllocation);
	buffer = nullptr;
	bufferAllocation = nullptr;
	bufferSize = 0;
}

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

	auto commandBuffer = beginSingleTimeCommands();
	vkCmdFillBuffer(commandBuffer.buffer, buffer, 0, VK_WHOLE_SIZE, 0);
	endSingleTimeCommands(commandBuffer);

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
