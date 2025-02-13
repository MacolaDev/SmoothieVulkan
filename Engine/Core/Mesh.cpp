#include "Mesh.h"
#include <vma/vk_mem_alloc.h>
#include "Core/VMA.h"
#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"
#include <iostream>

Mesh::Mesh(const std::string& file)
{
	filepath = file;
	if (isAlreadyLoaded(file))
	{

		Mesh* data = getResourse(file);
		vertexBuffer = data->vertexBuffer;
		vertexBufferAllocation = data->vertexBufferAllocation;
		vertexBufferAttributeType = data->vertexBufferAttributeType;

		indexBuffer = data->indexBuffer;
		indexBufferAllocation = data->indexBufferAllocation;
		indexType = data->indexType;
		numberOfIndices = data->numberOfIndices;

		increaseReferenceCount(file);
		return;
	}
	else
	{
		create(filepath);
		setResource(file, *this);
		return;
	}
}


void Mesh::create(const std::string& filepath)
{
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
		std::cout << __FUNCTION__": Uknown vertex type: " << data.vertexBufferType << "; Result is undefined! " << std::endl;
	}

	VkBufferCreateInfo vertexBufferCreateInfo{};
	vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferCreateInfo.size = data.vertexData.size();
	vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	//Creating and copying data from file to buffer
	vmaCreateBuffer(VMA::getAllocator(), &vertexBufferCreateInfo, &allocInfo, &vertexBuffer, &vertexBufferAllocation, nullptr);
	vmaCopyMemoryToAllocation(VMA::getAllocator(), data.vertexData.data(), vertexBufferAllocation, 0, data.vertexData.size());



	VkBufferCreateInfo indexBufferCreateInfo{};
	indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexBufferCreateInfo.size = data.indexData.size();
	indexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	vmaCreateBuffer(VMA::getAllocator(), &indexBufferCreateInfo, &allocInfo, &indexBuffer, &indexBufferAllocation, nullptr);
	vmaCopyMemoryToAllocation(VMA::getAllocator(), data.indexData.data(), indexBufferAllocation, 0, data.indexData.size());
	numberOfIndices = data.numberOfIndices;
}

void Mesh::destroy()
{
	decreaseReferenceCount(filepath);
	if (getReferenceCout(filepath) == 0)
	{
		vmaDestroyBuffer(VMA::getAllocator(), indexBuffer, indexBufferAllocation);
		indexBuffer = nullptr;
		indexBufferAllocation = nullptr;

		vmaDestroyBuffer(VMA::getAllocator(), vertexBuffer, vertexBufferAllocation);
		vertexBuffer = nullptr;
		vertexBufferAllocation = nullptr;
		removeResource(filepath);
	}
}

