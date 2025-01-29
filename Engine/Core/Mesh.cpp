#include "Mesh.h"
#include <vma/vk_mem_alloc.h>
#include "Core/VMA.h"
#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"
#include <iostream>

Mesh::Mesh(const std::string& filepath): GeometryFile(filepath)
{
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	
	//******************** vertex buffer ***********************//
	if (vertexBufferType == 0) 
	{
		vertexBufferAttributeType = std::make_shared<xyznuvtb>();
	}
	else
	{
		std::cout << __FUNCTION__": Uknown vertex type: " << vertexBufferType << "; Result is undefined! " << std::endl;
	}

	VkBufferCreateInfo vertexBufferCreateInfo{};
	vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferCreateInfo.size = vertexData.size();
	vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	
	//Creating and copying data from file to buffer
	vmaCreateBuffer(VMA::getAllocator(), &vertexBufferCreateInfo, &allocInfo, &vertexBuffer, &vertexBufferAllocation, nullptr);
	vmaCopyMemoryToAllocation(VMA::getAllocator(), vertexData.data(), vertexBufferAllocation, 0, vertexData.size());
	
	//Vertex pipeline info
	pipelineVertexInputInfo = {};
	pipelineVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputInfo.vertexBindingDescriptionCount = 1;
	
	bindigDescription = vertexBufferAttributeType.get()->getBindingDescription();
	pipelineVertexInputInfo.pVertexBindingDescriptions = &bindigDescription;
	
	vertexAttributes = vertexBufferAttributeType.get()->getVertexAttributes();
	pipelineVertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size());
	pipelineVertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();
	//***********************************************************//



	//********************** index buffer ***********************//
	VkBufferCreateInfo indexBufferCreateInfo{};
	indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexBufferCreateInfo.size = indexData.size();
	indexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	indexType = VK_INDEX_TYPE_UINT32; //TODO: Change to 16byte int, 32 is too large for small models

	//Creating and copying data from file to buffer
	vmaCreateBuffer(VMA::getAllocator(), &indexBufferCreateInfo, &allocInfo, &indexBuffer, &indexBufferAllocation, nullptr);
	vmaCopyMemoryToAllocation(VMA::getAllocator(), indexData.data(), indexBufferAllocation, 0, indexData.size());
	
	
	//Index pipeline data
	pipelineIndexInputInfo = {};
	pipelineIndexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineIndexInputInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineIndexInputInfo.primitiveRestartEnable = VK_FALSE;
	//***********************************************************//
}

void Mesh::bind(VkCommandBuffer commandBuffer) const
{
	const VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, indexType);
}

void Mesh::destroy()
{
	vmaDestroyBuffer(VMA::getAllocator(), indexBuffer, indexBufferAllocation);
	indexBuffer = nullptr;
	indexBufferAllocation = nullptr;

	vmaDestroyBuffer(VMA::getAllocator(), vertexBuffer, vertexBufferAllocation);
	vertexBuffer = nullptr;
	vertexBufferAllocation = nullptr;
}

VkPipelineVertexInputStateCreateInfo Mesh::getPipelineVertexInput() const
{
	return pipelineVertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo Mesh::getPipelineIndexInput() const
{
	return pipelineIndexInputInfo;
}

unsigned int Mesh::getNumberOfIndices() const
{
	return numberOfIndices;
}
