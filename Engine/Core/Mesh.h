#pragma once
#include "VertexBuffer.h"
#include "ResourceManager/GeometryFile.h"
#include <string>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include "ResourceManager/ResourceHandler.h"

struct Mesh: private ResourceHandler<Mesh>
{
	std::string filepath;
	VkBuffer vertexBuffer = nullptr;
	VmaAllocation vertexBufferAllocation = nullptr;
	std::shared_ptr<VertexBufferBase> vertexBufferAttributeType;
	
	VkBuffer indexBuffer = nullptr;
	VmaAllocation indexBufferAllocation = nullptr;

	VkIndexType indexType = VK_INDEX_TYPE_UINT32;
	unsigned int numberOfIndices = 0;

	Mesh(const std::string& filepath);

	void create(const std::string& filepath);
	void destroy();

	Mesh() = default;
};


