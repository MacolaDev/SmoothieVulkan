#pragma once
#include "Core/RenderPass.h"


namespace Smoothie 
{

	namespace DeferredRendering 
	{

		//*************************************
		//color attachment
		//*************************************
		class Attachment_Color : public RenderPassAttachment_Base
		{
		public:
			VkFormat imageFormat = VK_FORMAT_UNDEFINED;

			Attachment_Color(VkFormat format) : imageFormat(format) {};
			Attachment_Color() = default;
			int create() override final;
		};



		//*************************************
		//Depth attachment
		//*************************************
		class Attachment_Depth : public RenderPassAttachment_Base
		{
		public:
			VkFormat imageFormat = VK_FORMAT_UNDEFINED;

			Attachment_Depth(VkFormat format) : imageFormat(format) {};
			Attachment_Depth() = default;
			int create() override final;
		};



		//*************************************
		//gBuffer render pass
		//*************************************
		class gBufferPass : public RenderPass_Base
		{
			VkDescriptorSet imagesDescriptorSet = nullptr;
			VkDescriptorPool imagesDescriptorPool = nullptr;
			VkDescriptorSetLayout imagesDescriptorSetLayout = nullptr;

		public:
			Attachment_Color gPosition, gNormal, gAlbedo, gMRAO;
			Attachment_Depth gDepth;

			VkSampler sampler = nullptr;
			gBufferPass() = default;

			int create() override;
			int resize_callback() override;
			void destroy() override;

			void bindPass(VkCommandBuffer commandBuffer, unsigned int ImageIndex) const override final;
			void unbindPass(VkCommandBuffer commandBuffer, unsigned int ImageIndex) const override final;

			inline VkDescriptorSet getImagesDescriptorSet() const { return imagesDescriptorSet; }
			inline VkDescriptorSetLayout getImagesDescriptorSetLayout() const { return imagesDescriptorSetLayout; }
			inline VkDescriptorPool getImagesDescriptorPool() const { return imagesDescriptorPool; }
		};



		//*************************************
		//HDR render pass
		//*************************************
		class HDRPass : public RenderPass_Base
		{
		public:
			Attachment_Color HDR;
			Attachment_Depth gDepth;

			HDRPass() = default;

			int create() override final;
			int resize_callback() override;
			void destroy() override final;

			void bindPass(VkCommandBuffer commandBuffer, unsigned int ImageIndex) const override final;
			void unbindPass(VkCommandBuffer commandBuffer, unsigned int ImageIndex) const override final;

		};
	}
}


