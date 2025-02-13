#pragma once

#include <vulkan/vulkan.h>
#include "Core/Image.h"

class Skybox
{
	static VkDescriptorPool descriptorPool;
	static VkDescriptorSet descriptorSet;
	static VkDescriptorSetLayout descriptorSetLayout;
	static VkPipeline pipeline;
	static VkPipelineLayout pipelineLayout;

public: 
	static void create(const Image& environmentImage);
	static void draw(VkCommandBuffer commandBuffer);
	static void destroy();

};

