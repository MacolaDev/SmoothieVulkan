#include "SwapChain.h"
#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"
#include <iostream>

VkSurfaceCapabilitiesKHR SwapChain::capabilities;
std::vector<VkSurfaceFormatKHR> SwapChain::formats;
std::vector<VkPresentModeKHR> SwapChain::presentModes;
VkSwapchainKHR SwapChain::swapChain = nullptr;
std::vector<VkImage> SwapChain::swapChainImages;
VkFormat SwapChain::swapChainImageFormat;
VkExtent2D SwapChain::swapChainExtent;
std::vector<VkImageView> SwapChain::swapChainImageViews;
std::vector<VkFramebuffer> SwapChain::defaultFramebuffers;
VkRenderPass SwapChain::defaultRenderPass = nullptr;

static void createDefaultRenderPass(VkRenderPass& renderPass, const VkFormat& format) 
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &renderPass);

}

void SwapChain::create(unsigned int width, unsigned int height)
{
	auto surface = SmoothieCore::getSurface();
	auto physicalDevice = SmoothieCore::getPhysicalDevice();
	
	//Get surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

	//Get surface format
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
	}

	//Get present mode
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
	}

	//Swap chain
	VkSurfaceFormatKHR surfaceFormat = { VK_FORMAT_B8G8R8A8_SRGB , VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;//VK_PRESENT_MODE_MAILBOX_KHR;
	VkExtent2D extent = { width, height};

	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	
	unsigned int graphicsIndex = SmoothieCore::getQueueFamilyGraphicsIndex();
	unsigned int presentIndex = SmoothieCore::getQueueFamilyPresentIndex();
	uint32_t queueFamilyIndices[] = {graphicsIndex, presentIndex};

	if (graphicsIndex != presentIndex)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	vkCreateSwapchainKHR(SmoothieCore::getDevice(), &createInfo, nullptr, &swapChain);

	vkGetSwapchainImagesKHR(SmoothieCore::getDevice(), swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(SmoothieCore::getDevice(), swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	//Swap chain image views
	swapChainImageViews.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(SmoothieCore::getDevice(), &createInfo, nullptr, &swapChainImageViews[i]);
	}

	//Default renderPass
	createDefaultRenderPass(defaultRenderPass, swapChainImageFormat);
	
	//Swap chain framebuffers
	defaultFramebuffers.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = defaultRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &swapChainImageViews[i];
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;
		vkCreateFramebuffer(SmoothieCore::getDevice(), &framebufferInfo, nullptr, &defaultFramebuffers[i]);
	}


}

void SwapChain::destroy()
{
	vkDestroyRenderPass(SmoothieCore::getDevice(), defaultRenderPass, nullptr);
	for (size_t i = 0; i < swapChainImages.size(); i++) 
	{
		vkDestroyFramebuffer(SmoothieCore::getDevice(), defaultFramebuffers[i], nullptr);
		vkDestroyImageView(SmoothieCore::getDevice(), swapChainImageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(SmoothieCore::getDevice(), swapChain, nullptr);
}

VkRenderPass SwapChain::getDefaultRenderPass()
{
	return defaultRenderPass;
}

VkSwapchainKHR SwapChain::getSwapChain()
{
	return swapChain;
}

VkFormat SwapChain::getSwapChainImageFormat()
{
	return swapChainImageFormat;
}

VkExtent2D SwapChain::getSwapChainExtent()
{
	return swapChainExtent;
}

VkFramebuffer SwapChain::getSwapChainFramebuffer(unsigned int index)
{
	return defaultFramebuffers[index];
}

VkImage SwapChain::getSwapChainImage(unsigned int index)
{
	return swapChainImages[index];
}
