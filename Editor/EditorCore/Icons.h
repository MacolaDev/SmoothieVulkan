#pragma once
#include "Smoothie.h"
#include "imgui.h"
#include <string>

namespace SmoothieEditor 
{
	struct Icon
	{
		VkDescriptorSet TextureID = nullptr;
		VkImage image = nullptr;
		VkImageView imageView = nullptr;
		VmaAllocation allocation = nullptr;

		int create(const std::string& path);
		void destroy();
	};

	struct Icons
	{

		Icon camera_icon;
		Icon object_select_icon;

		int create();
		void destroy();
	};
}

