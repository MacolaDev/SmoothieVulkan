#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <ResourceManager/ResourceHandler.h>
#include "vk_mem_alloc.h"

namespace Smoothie 
{
	class Texture2D : public ResourceHandler<Texture2D>
	{
	public:
		
		int create(const std::string& filepath);
		void destroy();

		Texture2D() = default;

		inline VkImageView getImageView() const {return imageView;}
		inline VkImage getImage() const{return image;}
		inline VmaAllocation getAllocation() const {return allocation;}


	private:
		std::string filepath;

		VkImage image = nullptr;
		VkImageView imageView = nullptr;
		VmaAllocation allocation = nullptr;
	};

	//Empty 2D 256x256 texture with general layout with value (0.69f, 0.69f, 0.69f, 1.0f). 
	//It's men to be used as placeholder texture for any sampler.
	class DefaultTexture2D 
	{
		VkImageView imageView = nullptr;
		VkImage image = nullptr;
		VmaAllocation allocation = nullptr;

	public:
		int create();
		void destroy();

		inline VkImageView getImageView() const { return imageView; }
		inline VkImage getImage() const { return image; }
		DefaultTexture2D() = default;
	};

}

