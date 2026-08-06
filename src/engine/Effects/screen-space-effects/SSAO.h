#pragma once
#include <Common.h>

namespace Smoothie 
{
	namespace DeferredRendering 
	{
		class SSAO
		{
			VkRenderPass renderPass = nullptr;
			VkFramebuffer framebuffer = nullptr;

			VkPipeline pipeline = nullptr;
			VkPipelineLayout pipelineLayout = nullptr;

			VkDescriptorSet descriptorSet = nullptr;
			VkDescriptorSetLayout descriptorSetLayout = nullptr;
			VkDescriptorPool descriptorPool = nullptr;

			VkImage noiseImage = nullptr;
			VkImageView noiseImageView = nullptr;
			VmaAllocation noiseImageAllocation = nullptr;
			
			// std::vector<SmoothieMath::Vector4> ssaoNoise;
			// std::vector<SmoothieMath::Vector4> ssaoKernel;

			VkBuffer kernelUBO = nullptr;
			VmaAllocation kernelUBOAllocation = nullptr;

		public:

			VkImageView gDepthImageView = nullptr;
			VkImageView gNormalImageView = nullptr;

			int create();
			int resize_callback();
			void draw(VkCommandBuffer commandBuffer, VkDescriptorSet drawClassDescriptor, unsigned int ImageIndex) const;
			void destroy();
		};
	}
}