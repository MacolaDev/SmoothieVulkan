#pragma once
#include <vulkan/vulkan.h>
#include "Core/Image.h"

class DeferredPostprocessing
{

	static VkPipeline pipeline;
	static VkPipelineLayout pipelineLayout;

	static VkDescriptorPool descriptorPool;
	static VkDescriptorSetLayout descriptorSetLayout;
	static VkDescriptorSet descriptorSet;

public:
	static void create(const Image& HDRImage, const Image& bloomImage);
	static void destroy();
	static void update();
	static void draw(VkCommandBuffer commandBuffer, int index);
};

