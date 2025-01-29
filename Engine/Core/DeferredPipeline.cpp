#include "DeferredPipeline.h"
#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"
#include <iostream>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include "VMA.h"

#include "SwapChain.h"
#include "Core/DeviceDependency.h"

static Image createImageFromData(unsigned int texWidth, unsigned int texHeight, VkFormat format)
{

	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	createImage.extent = { texWidth , texHeight , 1 };
	createImage.mipLevels = 1;
	createImage.arrayLayers = 1;
	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createImage.format = format;
	
	Image image;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.priority = 1.0f;
	
	vmaCreateImage(VMA::getAllocator(), &createImage, &allocInfo, &image.image, &image.allocation, &image.allocationInfo);

	VkImageViewCreateInfo createImageView{};
	createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createImageView.image = image.image;
	createImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createImageView.format = format;
	createImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createImageView.subresourceRange.levelCount = 1;
	createImageView.subresourceRange.layerCount = 1;
	
	vkCreateImageView(SmoothieCore::getDevice(), &createImageView, nullptr, &image.imageView);
	return image;
}

static Image createImageDepth(unsigned int texWidth, unsigned int texHeight, VkFormat format)
{
	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	createImage.extent = { texWidth , texHeight , 1 };
	createImage.mipLevels = 1;
	createImage.arrayLayers = 1;
	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createImage.format = format;
	
	Image image;

	VmaAllocationCreateInfo allocInfo = {};
	//allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.priority = 1.0f;
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	vmaCreateImage(VMA::getAllocator(), &createImage, &allocInfo, &image.image, &image.allocation, &image.allocationInfo);

	VkImageViewCreateInfo createImageView{};
	createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createImageView.image = image.image;
	createImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createImageView.format = format;
	createImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	createImageView.subresourceRange.baseMipLevel = 0;
	createImageView.subresourceRange.levelCount = 1;
	createImageView.subresourceRange.baseArrayLayer = 0;
	createImageView.subresourceRange.layerCount = 1;
	vkCreateImageView(SmoothieCore::getDevice(), &createImageView, nullptr, &image.imageView);
	return image;
}



void gBufferPass::create(unsigned int width, unsigned int height)
{
	//Position attachment
	VkAttachmentDescription gPositionDescription{};
	gPositionDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	gPositionDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	gPositionDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	gPositionDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	gPositionDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	gPositionDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	gPositionDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	gPositionDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//Normals
	VkAttachmentDescription gNormalDescription{};
	gNormalDescription = gPositionDescription;
	gNormalDescription.format = VK_FORMAT_R16G16B16A16_SFLOAT;

	//Albedo channel
	VkAttachmentDescription gAlbedoDescription{};
	gAlbedoDescription = gPositionDescription;
	gAlbedoDescription.format = VK_FORMAT_R8G8B8A8_SRGB;

	//Metalic, roughness, AO
	VkAttachmentDescription gMRAODescription{};
	gMRAODescription = gPositionDescription;
	gMRAODescription.format = VK_FORMAT_R8G8B8A8_UNORM;

	//Depth 
	const auto depthFormat = DeviceDependencies::getSupportedDepthFormat();
	VkAttachmentDescription gDepthDescription{};
	gDepthDescription.format = depthFormat;
	gDepthDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	gDepthDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	gDepthDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	gDepthDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	gDepthDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	gDepthDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	gDepthDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkAttachmentReference colorAttachmentRefs[4] =
	{
		{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, // Position
		{1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, // Normals
		{2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, // Albedo
		{3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }  // metalic, roughness, ao
	};

	VkAttachmentReference depthAttachmentRef =
	{
		4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};


	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 4;
	subpass.pColorAttachments = colorAttachmentRefs;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkAttachmentDescription attachments[5] = 
	{ 
		gPositionDescription, 
		gNormalDescription, 
		gAlbedoDescription, 
		gMRAODescription, 
		gDepthDescription 
	};

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 5;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;


	VkSubpassDependency dependency{};
	dependency.srcSubpass = 0;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = 0;

	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = &dependency;


	vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &renderPass);

	//gBuffer images
	gPosition = createImageFromData(width, height, VK_FORMAT_R32G32B32A32_SFLOAT);
	gNormal = createImageFromData(width, height, VK_FORMAT_R16G16B16A16_SFLOAT);
	gAlbedo = createImageFromData(width, height, VK_FORMAT_R8G8B8A8_SRGB);
	gMRAO = createImageFromData(width, height, VK_FORMAT_R8G8B8A8_UNORM);
	gDepth = createImageDepth(width, height, depthFormat);

	//gBuffer framebuffer
	VkImageView attachmentsImageViews[] =
	{
		gPosition.imageView,
		gNormal.imageView,
		gAlbedo.imageView,
		gMRAO.imageView,
		gDepth.imageView
	};

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = renderPass;
	createInfo.attachmentCount = 5;
	createInfo.pAttachments = attachmentsImageViews;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = 1;

	vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &framebuffer);

	//Render pass begin info
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.framebuffer = framebuffer;
	beginInfo.renderPass = renderPass;

	VkClearValue clearColor{};
	clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };

	VkClearValue depth{};
	depth.depthStencil.depth = 1.0f;

	static VkClearValue clearValues[] = { clearColor , clearColor , clearColor , clearColor , depth };
	beginInfo.clearValueCount = 5;
	beginInfo.pClearValues = clearValues;

	beginInfo.renderArea.extent = { width, height };
	beginInfo.renderArea.offset = { 0, 0 };
	
	//Viewport data
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(width);
	viewport.height = static_cast<float>(height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

}

void gBufferPass::destroy()
{
	
	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr);
	framebuffer = nullptr;

	gDepth.destroyImage();
	gMRAO.destroyImage();
	gAlbedo.destroyImage();
	gNormal.destroyImage();
	gPosition.destroyImage();

	vkDestroyRenderPass(SmoothieCore::getDevice(), renderPass, nullptr);
	renderPass = nullptr;
}

void gBufferPass::update(unsigned int width, unsigned int height)
{
	destroy();
	create(width, height);
}

void gBufferPass::beginPass(VkCommandBuffer commandBuffer) const
{
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void gBufferPass::endPass(VkCommandBuffer commandBuffer) const
{
	vkCmdEndRenderPass(commandBuffer);
}

gBufferPass DeferredPipeline::gBuffer;
Smoothie::Model DeferredPipeline::testModel;

void DeferredPipeline::create(unsigned int width, unsigned int height)
{
	gBuffer.create(width, height);
	SmoothieMath::Matrix4x4 modelMatrix;
	modelMatrix.transformMatrix({ 0.717276f, 0.000000f, 0.019547f }, {0, 0,0}, {1, 1, 1});
	testModel = Smoothie::Model("resources/DemoScene/House/House.smodel", modelMatrix);
}

void DeferredPipeline::destroy()
{
	testModel.destroy();
	gBuffer.destroy();
}

void DeferredPipeline::update(unsigned int width, unsigned int height)
{
	gBuffer.update(width, height);
}

void DeferredPipeline::draw(VkCommandBuffer commandBuffer, unsigned int imageIndex)
{
	gBuffer.beginPass(commandBuffer);
	testModel.bindAndDraw(commandBuffer);
	gBuffer.endPass(commandBuffer);

	
	VkMemoryBarrier memoryBarier{};
	memoryBarier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	memoryBarier.dstAccessMask = 0;
	
	VkImageMemoryBarrier imageMemoryBarier{};
	imageMemoryBarier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarier.dstAccessMask = VK_ACCESS_NONE;
	imageMemoryBarier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageMemoryBarier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	imageMemoryBarier.image = SwapChain::getSwapChainImage(imageIndex);
	imageMemoryBarier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarier.subresourceRange.levelCount = 1;
	imageMemoryBarier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		1, &memoryBarier,
		0, nullptr,
		1, &imageMemoryBarier);
}

gBufferPass DeferredPipeline::get_gBufferPass()
{
	return gBuffer;
}

VkRenderPass DeferredPipeline::get_gBufferRenderPass()
{
	return gBuffer.renderPass;
}
