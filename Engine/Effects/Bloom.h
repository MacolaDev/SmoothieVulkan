#pragma once
#include <vulkan/vulkan.h>
#include "Core/Effect.h"
#include "vk_mem_alloc.h"
#include <vector>

namespace Smoothie {

	namespace DeferredRendering
	{
		class Bloom: public Effect_Base
		{
			VkPipeline higlightPipeline = nullptr;
			VkPipelineLayout higlightPipelineLayout = nullptr;

			VkPipeline downsamplingPipeline = nullptr;
			VkPipelineLayout downsamplingPipelineLayout = nullptr;

			VkPipeline upsamplingPipeline = nullptr;
			VkPipelineLayout upsamplingPipelineLayout = nullptr;

			VkRenderPass renderPass = nullptr;

			struct MipChainData
			{
				unsigned int width = 1, height = 1;
				VkImage image = nullptr;
				VkImageView imageView = nullptr;
				VmaAllocation allocation = nullptr;
				VkDescriptorSet dwnDescriptorSet = nullptr;
				VkDescriptorSet upDescriptorSet = nullptr;
				VkDescriptorPool descriptorPool = nullptr;
				VkFramebuffer framebuffer = nullptr;
			};

			std::vector<MipChainData> mipChainData;

			VkFramebuffer higlight_Framebuffer = nullptr;
			VkImage higlight_TargetImage = nullptr;
			VkImageView higlight_TargetImageView = nullptr;
			VmaAllocation higlight_TargetAllocation = nullptr;

			VkDescriptorSetLayout descriptorSetLayout = nullptr;
			VkDescriptorPool descriptorPool = nullptr;

		public:
		
			VkImage HDRImage = nullptr;
			VkImageView HDRImageView = nullptr;

			VkSampler ClampToEdgeLINEAR = nullptr;

			VkShaderModule vertexShader = nullptr;
			VkShaderModule higlightModule = nullptr;
			VkShaderModule downsampleModule = nullptr;
			VkShaderModule upsampleModule = nullptr;

			int create() override;
			void draw(VkCommandBuffer commandBuffer, VkDescriptorSet drawClassDescriptor, unsigned int ImageIndex) const override;
			void destroy() override;
			int resize_callback() override;

			VkImageView getFinalImage() const;

			Bloom() = default;
		};

	}
}

