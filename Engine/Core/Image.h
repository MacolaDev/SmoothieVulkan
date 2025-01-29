#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <string>
#include "Core/VMA.h"

struct Image
{
	VkBuffer imageBuffer = nullptr;
	VmaAllocationInfo allocationInfo{};
	VmaAllocation allocation = nullptr;
	VkImage image = nullptr;
	VkImageView imageView = nullptr;

	void destroyImage();
};

class Texture2D: public Image
{	
public:
	Texture2D() = default;
	Texture2D(const std::string& filepath);
	
private:
	int width = 1024, height = 1024;
	int nOfChannels = 3;
	std::string filepath;
};


class SmoothieCore;
class Samplers 
{
public:
	static VkSampler Texture2DModelSampler;


private:
	static void create();
	static void destroy();
	friend class SmoothieCore;
};