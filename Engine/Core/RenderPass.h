#pragma once
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <string>
#include <memory>
#include "Core/Pipeline.h"
#include "vk_mem_alloc.h"

namespace Smoothie 
{
	
	//Base Attachment for images used as targets in render pass
	class RenderPassAttachment_Base
	{
	protected:
		VkImage image = nullptr;
		VkImageView imageView = nullptr;
		VmaAllocation imageAllocation = nullptr;
	public:
		RenderPassAttachment_Base() = default;
		virtual int create() = 0;
		void destroy();

		inline VkImage getImage() const { return image; }
		inline VkImageView getImageView() const { return imageView; }
		inline VmaAllocation getImageAllocation() const { return imageAllocation; }

	};
	
	//Base class for data needed to render one render pass
	//It's instances should contain framebuffer, images, render pass object and pipelines that can be used in this render pass
	class RenderPass_Base 
	{
	protected:
		VkRenderPass render_pass = nullptr; 
		VkFramebuffer framebuffer = nullptr;

	public:
		RenderPass_Base() = default;

		virtual int create() = 0;
		virtual int resize_callback() = 0;
		virtual void destroy() = 0;
		
		virtual void bindPass(VkCommandBuffer commandBuffer, unsigned int ImageIndex) const= 0;
		virtual void unbindPass(VkCommandBuffer commandBuffer, unsigned int ImageIndex) const = 0;

		inline VkRenderPass get_render_pass() const { return render_pass; }
		inline VkFramebuffer get_framebuffer() const { return framebuffer; }
	};




}