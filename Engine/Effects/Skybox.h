#pragma once
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include <string>

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
			VkShaderModule pbsVertexShader = nullptr;
			VkShaderModule fragmentShader = nullptr;

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

		class Skybox
		{
			VkPipeline pipeline = nullptr;
			VkPipelineLayout pipelineLayout = nullptr;
		public:
			VkDescriptorSet HDRCubemap_descriptorSet = nullptr;
			VkDescriptorSetLayout HDRCubemap_descriptorSetLayout = nullptr;

			VkRenderPass renderPass = nullptr;
			VkDescriptorSetLayout drawerClassDescriptorSetLayout = nullptr;

			VkShaderModule skyboxVertex = nullptr;
			VkShaderModule skyboxFragment = nullptr;

			int create();
			void draw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, unsigned int ImageID) const;
			void destroy();

		};

	}
}

