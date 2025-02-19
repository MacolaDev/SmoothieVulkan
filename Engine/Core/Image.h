#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <string>
#include "Core/VMA.h"
#include <array>
#include "ResourceManager/ResourceHandler.h"



void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

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

//Model textures
class Texture2D: private ResourceHandler<Texture2D>
{	
public:
	Image image;

	void create(const std::string& filepath);
	void destroy();
	Texture2D(const std::string& filepath);
	Texture2D() = default;

private:
	std::string filepath;
};


class SmoothieCore;
class Samplers 
{
public:
	static VkSampler Texture2DModelSampler;
	static VkSampler ClampToEdgeLINEAR;

private:
	static void create();
	static void destroy();
	friend class SmoothieCore;
};