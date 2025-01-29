#pragma once
#include <vulkan/vulkan.h>
#include "Image.h"
#include <vector>
#include <Core/Model.h>

struct gBufferPass 
{
	VkRenderPass renderPass = nullptr;
	VkFramebuffer framebuffer = nullptr;
	Image gPosition, gNormal, gAlbedo, gMRAO, gDepth;
	VkRenderPassBeginInfo beginInfo{};
	VkViewport viewport{};

	void create(unsigned int width, unsigned int height);
	void destroy();
	void update(unsigned int width, unsigned int height);

	void beginPass(VkCommandBuffer commandBuffer) const;
	void endPass(VkCommandBuffer commandBuffer) const;
};


class DeferredPipeline
{
	static gBufferPass gBuffer;
public:

	static void create(unsigned int width, unsigned int height);
	static void destroy();
	static void update(unsigned int width, unsigned int height);
	static void draw(VkCommandBuffer commandBuffer, unsigned int swapchainTarget);
	static Smoothie::Model testModel;
	static gBufferPass get_gBufferPass();
	static VkRenderPass get_gBufferRenderPass();
};

