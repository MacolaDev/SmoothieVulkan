#pragma once
#include <vulkan/vulkan.h>
#include "Core/Camera.h"
#include "Core/VMA.h"

struct CameraDescriptor
{
	static VkDescriptorSet descriptorSet;
	static VkDescriptorSetLayout descriptorSetLayout;
	static VkDescriptorPool descriptorPool;
	static VkBuffer buffer;
	static VmaAllocation allocation;

	static void create();
	static void update(const CameraUniformBufferData& data);
	static void destroy();

};