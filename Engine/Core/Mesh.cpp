#include "Mesh.h"
#include <iostream>
#include <mutex>
#include "Core/SmoothieCore.h"

static std::mutex mutex;
using namespace Smoothie;
using namespace Depricated;
int Mesh::create(const std::string& filepath)
{
	this->filepath = filepath;
	std::lock_guard<std::mutex> lock(mutex);

	GeometryFile data = GeometryFile(filepath);
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

	//******************** vertex buffer ***********************//
	if (data.vertexBufferType == 0)
	{
		vertexBufferAttributeType = std::make_shared<xyznuvtb>();
	}
	else if (data.vertexBufferType == 1)
	{
		vertexBufferAttributeType = std::make_shared<xyznuvtbc>();
	}
	else
	{
		std::cout << "Uknown vertex type: " << data.vertexBufferType << "; Result is undefined! " << std::endl;
	}

	VkBufferCreateInfo vertexBufferCreateInfo{};
	vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferCreateInfo.size = data.vertexData.size();
	vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	//Creating and copying data from file to buffer
	vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &vertexBufferCreateInfo, &allocInfo, &vertexBuffer, &vertexBufferAllocation, nullptr);
	std::string vertexBufferAllocationName = "Vertex buffer: " + filepath;
	vmaSetAllocationName(SmoothieCore::getVulkanMemoryAllocator(), vertexBufferAllocation, vertexBufferAllocationName.c_str());
	vmaCopyMemoryToAllocation(SmoothieCore::getVulkanMemoryAllocator(), data.vertexData.data(), vertexBufferAllocation, 0, data.vertexData.size());


	VkBufferCreateInfo indexBufferCreateInfo{};
	indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexBufferCreateInfo.size = data.indexData.size();
	indexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &indexBufferCreateInfo, &allocInfo, &indexBuffer, &indexBufferAllocation, nullptr);

	std::string indexBufferAllocationName = "Index buffer: " + filepath;
	vmaSetAllocationName(SmoothieCore::getVulkanMemoryAllocator(), indexBufferAllocation, indexBufferAllocationName.c_str());

	vmaCopyMemoryToAllocation(SmoothieCore::getVulkanMemoryAllocator(), data.indexData.data(), indexBufferAllocation, 0, data.indexData.size());
	numberOfIndices = data.numberOfIndices;

	return 0;
}

void Mesh::destroy()
{
	vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), indexBuffer, indexBufferAllocation);
	indexBuffer = nullptr, indexBufferAllocation = nullptr;

	vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), vertexBuffer, vertexBufferAllocation);
	vertexBuffer = nullptr, vertexBufferAllocation = nullptr;
}

