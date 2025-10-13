#include "RenderPass.h"
#include "Core/DeviceDependency.h"
#include "Core/SmoothieCore.h"

static void destroyPass(VkRenderPass& renderPass) 
{
	vkDestroyRenderPass(SmoothieCore::getDevice(), renderPass, nullptr);
	renderPass = nullptr;
}

void Smoothie::RenderPassAttachment_Base::destroy()
{
	vkDestroyImageView(SmoothieCore::getDevice(), imageView, nullptr);
	imageView = nullptr;

	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), image, imageAllocation);
	image = nullptr, imageAllocation = nullptr;
}
