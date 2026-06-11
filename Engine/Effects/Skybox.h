#pragma once
#include <Core/Common.h>

namespace Smoothie 
{

	namespace DeferredRendering
	{
		class SkyboxCubemapTexture
		{
			VkImage image = nullptr;
			VkImageView imageView = nullptr;
			VmaAllocation allocation = nullptr;

			VkDescriptorSet descriptorSet = nullptr;
			VkDescriptorSetLayout descriptorSetLayout = nullptr;
			VkDescriptorPool descriptorPool = nullptr;

		public:
			VkSampler Texture2DModelSampler = nullptr;
			VkShaderModule m_FragmentShaderModule = nullptr;
			VkShaderModule m_VertexShaderModule = nullptr;

			int create();
			//Creates cubemap texture from a hdri 
			int create_from_hdri_image(const std::string& filepath);
			void destroy();

			inline VkImage getImage() const { return image; }
			inline VkImageView getImageView() const { return imageView; }
			inline VmaAllocation getVmaAllocation() const { return allocation; }
			
			inline VkDescriptorSet getDescriptorSet() const { return descriptorSet; }
			inline VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
		};

	}
}

