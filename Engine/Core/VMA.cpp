#include "VMA.h"

#include <vulkan/vulkan.h>
#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"
#include <iostream>


VmaAllocator VMA::allocator = nullptr;

void VMA::createAllocator()
{
	VmaVulkanFunctions vulkanFunctions = {};
	vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
	allocatorCreateInfo.physicalDevice = SmoothieCore::getPhysicalDevice();
	allocatorCreateInfo.device = SmoothieCore::getDevice();
	allocatorCreateInfo.instance = SmoothieCore::getInstance();
	allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
	
	vmaCreateAllocator(&allocatorCreateInfo, &allocator);
	
}

void VMA::freeAllocator()
{
	vmaDestroyAllocator(allocator);
}

VmaAllocator VMA::getAllocator()
{
	return allocator;
}
