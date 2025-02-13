#include "DeferredPipeline.h"
#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"
#include <iostream>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include "VMA.h"

#include "SwapChain.h"
#include "Core/DeviceDependency.h"
#include "RenderPass.h"
#include "Core/RenderPass.h"
#include "Effects/Skybox.h"
#include "Effects/Bloom.h"
#include "Effects/DeferredPostprocessing.h"

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
	createImage.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
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

void gBuffer::translateAttachmentToShaderReadOnly(VkCommandBuffer commandBuffer) const
{
	VkImageMemoryBarrier imageMemoryBarier{};
	imageMemoryBarier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarier.image = gPosition.image;
	imageMemoryBarier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarier.subresourceRange.levelCount = 1;
	imageMemoryBarier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarier.subresourceRange.layerCount = 1;

	VkImageMemoryBarrier gPositionBarrier = imageMemoryBarier;
	gPositionBarrier.image = gPosition.image;

	VkImageMemoryBarrier gNormalBarrier = imageMemoryBarier;
	gNormalBarrier.image = gNormal.image;

	VkImageMemoryBarrier gAlbedoBarrier = imageMemoryBarier;
	gAlbedoBarrier.image = gAlbedo.image;

	VkImageMemoryBarrier gMRAOBarrier = imageMemoryBarier;
	gMRAOBarrier.image = gMRAO.image;

	VkImageMemoryBarrier gDepthBarrier = imageMemoryBarier;
	gDepthBarrier.image = gDepth.image;
	gDepthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	
	VkImageMemoryBarrier imageMemoryBariers[] = { gPositionBarrier, gNormalBarrier, gAlbedoBarrier, gMRAOBarrier, gDepthBarrier };
	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_DEPENDENCY_BY_REGION_BIT,
		0, nullptr,
		0, nullptr,
		4, imageMemoryBariers);
}

void gBuffer::translateShaderReadOnlyToAttachment(VkCommandBuffer commandBuffer) const
{
	VkImageMemoryBarrier imageMemoryBarier{};
	imageMemoryBarier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarier.image = gPosition.image;
	imageMemoryBarier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarier.subresourceRange.levelCount = 1;
	imageMemoryBarier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarier.subresourceRange.layerCount = 1;

	VkImageMemoryBarrier gPositionBarrier = imageMemoryBarier;
	gPositionBarrier.image = gPosition.image;

	VkImageMemoryBarrier gNormalBarrier = imageMemoryBarier;
	gNormalBarrier.image = gNormal.image;

	VkImageMemoryBarrier gAlbedoBarrier = imageMemoryBarier;
	gAlbedoBarrier.image = gAlbedo.image;

	VkImageMemoryBarrier gMRAOBarrier = imageMemoryBarier;
	gMRAOBarrier.image = gMRAO.image;

	VkImageMemoryBarrier gDepthBarrier = imageMemoryBarier;
	gDepthBarrier.image = gDepth.image;
	gDepthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	VkImageMemoryBarrier imageMemoryBariers[] = { gPositionBarrier, gNormalBarrier, gAlbedoBarrier, gMRAOBarrier, gDepthBarrier };
	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_DEPENDENCY_BY_REGION_BIT,
		0, nullptr,
		0, nullptr,
		4, imageMemoryBariers);
}

void gBuffer::create(unsigned int width, unsigned int height)
{

	//gBuffer images
	gPosition = createImageFromData(width, height, VK_FORMAT_R32G32B32A32_SFLOAT);
	gNormal = createImageFromData(width, height, VK_FORMAT_R16G16B16A16_SFLOAT);
	gAlbedo = createImageFromData(width, height, VK_FORMAT_R8G8B8A8_UNORM);
	gMRAO = createImageFromData(width, height, VK_FORMAT_R8G8B8A8_UNORM);
	const auto depthFormat = DeviceDependencies::getSupportedDepthFormat();
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
	createInfo.renderPass = gBufferPass::renderPass;
	createInfo.attachmentCount = 5;
	createInfo.pAttachments = attachmentsImageViews;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = 1;

	vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &framebuffer);

	//Render pass begin info
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.framebuffer = framebuffer;
	beginInfo.renderPass = gBufferPass::renderPass;

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

void gBuffer::destroy()
{
	
	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr);
	framebuffer = nullptr;

	gDepth.destroyImage();
	gMRAO.destroyImage();
	gAlbedo.destroyImage();
	gNormal.destroyImage();
	gPosition.destroyImage();
}

void gBuffer::update(unsigned int width, unsigned int height)
{
	destroy();
	create(width, height);
}

void gBuffer::beginPass(VkCommandBuffer commandBuffer) const
{
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void gBuffer::endPass(VkCommandBuffer commandBuffer) const
{
	vkCmdEndRenderPass(commandBuffer);
}

GraphicsPass1 DeferredPipeline::pass1;

void DeferredPipeline::create(unsigned int width, unsigned int height)
{
	pass1.create(width, height);
}

void DeferredPipeline::destroy()
{
	pass1.destroy();
}

void DeferredPipeline::update(unsigned int width, unsigned int height)
{
	pass1.update(width, height);
}

std::list<Smoothie::Model*> DeferredPipeline::PBRModels;
std::list<Smoothie::Model*> DeferredPipeline::HDRModels;

void DeferredPipeline::draw(VkCommandBuffer commandBuffer, unsigned int imageIndex)
{
	//PBR pass
	pass1._gBufferPass.beginPass(commandBuffer);
	for (auto model : PBRModels) 
	{
		model->bindAndDraw(commandBuffer);
	}
	pass1._gBufferPass.endPass(commandBuffer);

	//Screen space effects with gBuffer data
	pass1._gBufferPass.translateAttachmentToShaderReadOnly(commandBuffer);
	
	pass1.pbr.draw(commandBuffer);

	pass1._gBufferPass.translateShaderReadOnlyToAttachment(commandBuffer);
	
	//HDR pass
	pass1._HDRPass.beginPass(commandBuffer);
	Skybox::draw(commandBuffer);
	for (auto model : HDRModels)
	{
		model->bindAndDraw(commandBuffer);
	}
	pass1._HDRPass.endPass(commandBuffer);
	
	//Post processing effects
	pass1._HDRPass.translateAttachmentToShaderReadOnly(commandBuffer);
	Bloom::bindAndDraw(commandBuffer);
	DeferredPostprocessing::draw(commandBuffer, imageIndex);
	pass1._HDRPass.translateShaderReadOnlyToAttachment(commandBuffer);
	
}

void GraphicsPass1::create(unsigned int width, unsigned int height)
{
	_gBufferPass.create(width, height);
	_HDRPass.create(width, height, &_gBufferPass.gDepth);
	pbr.create(width, height, _gBufferPass, _HDRPass.HDRImage.imageView);
	Bloom::create(width, height, _HDRPass.HDRImage);
	const auto bloomImage = Bloom::getBloomImage();
	DeferredPostprocessing::create(_HDRPass.HDRImage, bloomImage);
}

void GraphicsPass1::destroy()
{
	DeferredPostprocessing::destroy();
	Bloom::destroy();
	pbr.destroy();
	_HDRPass.destroy();
	_gBufferPass.destroy();
}

void GraphicsPass1::update(unsigned int width, unsigned int height)
{

}

void GraphicsPass1::bind(VkCommandBuffer commandBuffer) const
{

}

void HDR::create(unsigned int width, unsigned int height, Image* gBufferDepthImage)
{
	HDRImage = createImageFromData(width, height, VK_FORMAT_R16G16B16A16_SFLOAT);

	VkImageView attachmentsImageViews[] =
	{
		HDRImage.imageView, gBufferDepthImage->imageView
	};

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = HDRPass::renderPass;
	createInfo.attachmentCount = 2;
	createInfo.pAttachments = attachmentsImageViews;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = 1;

	vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &framebuffer);

	//Render pass begin info
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.framebuffer = framebuffer;
	beginInfo.renderPass = HDRPass::renderPass;

	VkClearValue clearColor{};
	clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };

	VkClearValue depth{};
	depth.depthStencil.depth = 1.0f;

	static VkClearValue clearValues[] = { clearColor, depth };
	beginInfo.clearValueCount = 2;
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

void HDR::destroy()
{

	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr);
	framebuffer = nullptr;
	HDRImage.destroyImage();
}

void HDR::update(unsigned int width, unsigned int height)
{

}

void HDR::beginPass(VkCommandBuffer commandBuffer) const
{
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void HDR::endPass(VkCommandBuffer commandBuffer) const
{
	vkCmdEndRenderPass(commandBuffer);
}

void HDR::translateAttachmentToShaderReadOnly(VkCommandBuffer commandBuffer) const
{
	VkImageMemoryBarrier imageMemoryBarier{};
	imageMemoryBarier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarier.image = HDRImage.image;
	imageMemoryBarier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarier.subresourceRange.levelCount = 1;
	imageMemoryBarier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_DEPENDENCY_BY_REGION_BIT,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarier);
}

void HDR::translateShaderReadOnlyToAttachment(VkCommandBuffer commandBuffer) const
{
	VkImageMemoryBarrier imageMemoryBarier{};
	imageMemoryBarier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarier.image = HDRImage.image;
	imageMemoryBarier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarier.subresourceRange.levelCount = 1;
	imageMemoryBarier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarier.subresourceRange.layerCount = 1;


	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_DEPENDENCY_BY_REGION_BIT,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarier);
}
