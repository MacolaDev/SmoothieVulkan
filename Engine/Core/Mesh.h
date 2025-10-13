#pragma once
#include "VertexBuffer.h"
#include "ResourceManager/GeometryFile.h"
#include <string>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include "ResourceManager/ResourceHandler.h"
#include "vk_mem_alloc.h"


namespace Smoothie 
{

	class Mesh : public ResourceHandler<Mesh>
	{
	public:
		int create(const std::string& filepath) override;
		void destroy() override;
		std::string filepath;
		VkBuffer vertexBuffer = nullptr;
		VmaAllocation vertexBufferAllocation = nullptr;
		std::shared_ptr<Depricated::VertexBufferBase> vertexBufferAttributeType;

		VkBuffer indexBuffer = nullptr;
		VmaAllocation indexBufferAllocation = nullptr;

		VkIndexType indexType = VK_INDEX_TYPE_UINT32;
		unsigned int numberOfIndices = 0;
			
		Mesh() = default;
	};


}