#pragma once
#include <vulkan/vulkan.h>
#include "Image.h"
#include <vector>
#include "Core/Model.h"
#include "Effects/PBR.h"
#include <list>

struct gBuffer
{
	VkFramebuffer framebuffer = nullptr;
	Image gPosition, gNormal, gAlbedo, gMRAO, gDepth;

	VkRenderPassBeginInfo beginInfo{};
	VkViewport viewport{};

	void translateAttachmentToShaderReadOnly(VkCommandBuffer commandBuffer) const;
	void translateShaderReadOnlyToAttachment(VkCommandBuffer commandBuffer) const;

	void create(unsigned int width, unsigned int height);
	void destroy();
	void update(unsigned int width, unsigned int height);

	void beginPass(VkCommandBuffer commandBuffer) const;
	void endPass(VkCommandBuffer commandBuffer) const;
};

struct HDR 
{
	VkFramebuffer framebuffer = nullptr;

	Image HDRImage;
	VkRenderPassBeginInfo beginInfo{};
	VkViewport viewport{};

	void create(unsigned int width, unsigned int height, Image* gBufferDepthImage);
	void destroy();
	void update(unsigned int width, unsigned int height);

	void beginPass(VkCommandBuffer commandBuffer) const;
	void endPass(VkCommandBuffer commandBuffer) const;

	void translateAttachmentToShaderReadOnly(VkCommandBuffer commandBuffer) const;
	void translateShaderReadOnlyToAttachment(VkCommandBuffer commandBuffer) const;
};

struct GraphicsPass1
{
	gBuffer _gBufferPass;
	HDR _HDRPass;
	PBR pbr;

	void create(unsigned int width, unsigned int height);
	void destroy();
	void update(unsigned int width, unsigned int height);
	void bind(VkCommandBuffer commandBuffer) const;
};

class DeferredPipeline
{
	static GraphicsPass1 pass1;


public:

	static void create(unsigned int width, unsigned int height);
	static void destroy();
	static void update(unsigned int width, unsigned int height);
	static void draw(VkCommandBuffer commandBuffer, unsigned int swapchainTarget);

	static std::list<Smoothie::Model*> PBRModels;
	static std::list<Smoothie::Model*> HDRModels;
};

