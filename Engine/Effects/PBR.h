#pragma once
#include "Core/Image.h"
#include <vulkan/vulkan.h>

struct gBuffer;
class PBR
{
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;

	VkDescriptorPool descriptorPool = nullptr;
	VkDescriptorSet descriptorSet = nullptr;
	VkDescriptorSetLayout descriptorSetLayout = nullptr;

	VkFramebuffer framebuffer = nullptr;
public:
	void create(unsigned int width, unsigned int height, const gBuffer& gBuffrerData, const VkImageView& HDRRenderTarget);
	void destroy();
	void update(unsigned int width, unsigned int height, const gBuffer& gBuffrerData, const VkImageView& HDRRenderTarget);

	void draw(VkCommandBuffer commandBuffer) const;

	PBR() = default;
};