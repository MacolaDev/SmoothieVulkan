#pragma once
#include "Smoothie.h"
#include "imgui.h"
#include <string>
#include <utility>

namespace SmoothieEditor 
{
	struct Icon
	{
		const std::string path;
		VkSampler sampler = nullptr;
	public:
		explicit Icon(std::string path, VkSampler sampler): path(std::move(path)), sampler(sampler) {};

		VkDescriptorSet TextureID = nullptr;
		VkImage image = nullptr;
		VkImageView imageView = nullptr;
		VmaAllocation allocation = nullptr;

		int create();
		void destroy();
	};

}

