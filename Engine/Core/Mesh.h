#pragma once
#include "VertexBuffer.h"
#include "ResourceManager/GeometryFile.h"
#include <string>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

class Mesh: private GeometryFile
{
	std::shared_ptr<VertexBufferBase> vertexBufferAttributeType;
	VkBuffer vertexBuffer = nullptr;
	VmaAllocation vertexBufferAllocation = nullptr;
	VkPipelineVertexInputStateCreateInfo pipelineVertexInputInfo;

	std::vector<VkVertexInputAttributeDescription> vertexAttributes;
	VkVertexInputBindingDescription bindigDescription;

	VkBuffer indexBuffer = nullptr;
	VmaAllocation indexBufferAllocation = nullptr;
	VkPipelineInputAssemblyStateCreateInfo pipelineIndexInputInfo;
	VkIndexType indexType = VK_INDEX_TYPE_UINT32;

public:
	Mesh(const std::string& filepath);
	void bind(VkCommandBuffer commandBuffer) const;
	void destroy();

	VkPipelineVertexInputStateCreateInfo getPipelineVertexInput() const;
	VkPipelineInputAssemblyStateCreateInfo getPipelineIndexInput() const;
	Mesh() = default;

	unsigned int getNumberOfIndices() const;
};

