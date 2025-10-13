#pragma once
#include <vulkan/vulkan.h>
#include <string>

#include <array>
#include "ResourceManager/ResourceHandler.h"
#include "Core/Constants.h"
#include "vk_mem_alloc.h"

void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, unsigned int mipLevelCount = 1);
void generateMipmaps(VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

//Used for everything that needs textures
struct Image
{
	VkImage image = nullptr;
	VkImageView imageView = nullptr;

	VkBuffer imageBuffer = nullptr;
	VmaAllocationInfo allocationInfo{};
	VmaAllocation allocation = nullptr;

	void destroyImage();
};
