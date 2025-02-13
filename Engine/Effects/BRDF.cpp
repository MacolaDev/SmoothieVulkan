#include "BRDF.h"
#define _SMOOTHIE_ENGINE
#include "Core/SmoothieCore.h"
#include "Core/PostProcessing.h"
#include "Core/Shader.h"

#include "stb_image.h"
#include <iostream>
#include <array>
#include "Math/SmoothieMath.h"
using namespace SmoothieMath;
Image BRDF::image;

static void createSinglePassRenderPass(VkRenderPass& renderPass, const VkFormat& format)
{
	VkAttachmentDescription attachmentDescription{};
	attachmentDescription.format = format;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference attachmentRefrence{};
	attachmentRefrence.attachment = 0;
	attachmentRefrence.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &attachmentRefrence;
	subpass.pDepthStencilAttachment = nullptr;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &attachmentDescription;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &renderPass);
}

static void createFramebuffer(const VkRenderPass& renderPass, const VkImageView& imageView, VkFramebuffer& frameBuffer, unsigned int width, unsigned int height) 
{
	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = renderPass;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &imageView;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = 1;

	vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &frameBuffer);
}

static void populateRenderPassBeginInfo(VkRenderPassBeginInfo& beginInfo, const VkFramebuffer& framebuffer, const VkRenderPass& renderPass) 
{
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.framebuffer = framebuffer;
	beginInfo.renderPass = renderPass;

	VkClearValue clearColor{};
	clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };

	beginInfo.clearValueCount = 1;
	beginInfo.pClearValues = &clearColor;

	beginInfo.renderArea.extent = { 512, 512 };
	beginInfo.renderArea.offset = { 0, 0 };
}

static void beginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo& beginInfo) 
{
	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(512);
	viewport.height = static_cast<float>(512);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

static void endRenderPass(VkCommandBuffer commandBuffer, const VkImage& imageView) 
{
	vkCmdEndRenderPass(commandBuffer);

	VkImageMemoryBarrier imageMemoryBarier{};
	imageMemoryBarier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarier.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	imageMemoryBarier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarier.image = imageView;
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

static void createPipelineData(VkPipelineLayout& pipelineLayout, VkPipeline& pipeline, const VkRenderPass& renderPass) 
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

	//Pipeline creation
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;


	//Shader stages
	VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
	vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderPipelineCreateInfo.pName = "main";
	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderPipelineCreateInfo.module = PostProcessingShaders::basicVertexShader;

	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderPipelineCreateInfo.pName = "main";
	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderPipelineCreateInfo.module = PostProcessingShaders::BRDF;

	VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;

	pipelineInfo.pVertexInputState = &PostProcessing::standardVertexState;
	pipelineInfo.pInputAssemblyState = &PostProcessing::standardAssemblyState;
	pipelineInfo.pViewportState = &PostProcessing::viewportState;
	pipelineInfo.pRasterizationState = &PostProcessing::rasterizer;
	pipelineInfo.pDepthStencilState = &PostProcessing::pipelineDepthStencil;
	pipelineInfo.pMultisampleState = &PostProcessing::multisampling;
	pipelineInfo.pColorBlendState = &PostProcessing::colorBlending;
	pipelineInfo.pDynamicState = &PostProcessing::dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;

	vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

}

static void destroyPipelineData(VkPipelineLayout& pipelineLayout, VkPipeline& pipeline) 
{
	vkDestroyPipeline(SmoothieCore::getDevice(), pipeline, nullptr);
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), pipelineLayout, nullptr);
	pipelineLayout = nullptr;
	pipeline = nullptr;
}

static void draw(VkCommandBuffer commandBuffer, const VkPipeline& pipeline, const VkPipelineLayout pipelineLayout) 
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(512);
	viewport.height = static_cast<float>(512);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { 512, 512 };
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdDraw(commandBuffer, 6, 1, 0, 0);
}

void BRDF::create()
{
	
	createImage();
	
	//Create render pass, framebuffer, render pass begin info and pipeline
	VkRenderPass renderPass = nullptr;
	createSinglePassRenderPass(renderPass, VK_FORMAT_R16G16_SFLOAT);
	VkFramebuffer framebuffer = nullptr;
	createFramebuffer(renderPass, image.imageView, framebuffer, 512, 512);
	VkRenderPassBeginInfo renderPassBeginInfo{};
	populateRenderPassBeginInfo(renderPassBeginInfo, framebuffer, renderPass);
	VkPipelineLayout pipelineLayout = nullptr;
	VkPipeline pipeline = nullptr;
	createPipelineData(pipelineLayout, pipeline, renderPass);

	//Begin command buffer, start render pass, draw 
	auto commandBuffer = beginSingleTimeCommands();
	beginRenderPass(commandBuffer.buffer, renderPassBeginInfo);
	
	draw(commandBuffer.buffer, pipeline, pipelineLayout);

	//End render pass and command buffer
	endRenderPass(commandBuffer.buffer, image.image);
	endSingleTimeCommands(commandBuffer);

	//Free resources used for generating brdf
	destroyPipelineData(pipelineLayout, pipeline);
	vkDestroyRenderPass(SmoothieCore::getDevice(), renderPass, nullptr);
	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr);
}

void BRDF::destroy()
{
	image.destroyImage();
}

void BRDF::createImage()
{
	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	createImage.extent = { 512 , 512 , 1 };
	createImage.mipLevels = 1;
	createImage.arrayLayers = 1;
	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createImage.format = VK_FORMAT_R16G16_SFLOAT;
	
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.priority = 1.0f;
	
	vmaCreateImage(VMA::getAllocator(), &createImage, &allocInfo, &image.image, &image.allocation, &image.allocationInfo);

	VkImageViewCreateInfo createImageView{};
	createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createImageView.image = image.image;
	createImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createImageView.format = VK_FORMAT_R16G16_SFLOAT;
	createImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createImageView.subresourceRange.levelCount = 1;
	createImageView.subresourceRange.layerCount = 1;

	vkCreateImageView(SmoothieCore::getDevice(), &createImageView, nullptr, &image.imageView);
}

static void getHdriImage(const std::string& filepath, Image& image)
{
	//HDRI image loading
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float* data = stbi_loadf(filepath.c_str(), &width, &height, &nrComponents, 4);

	if (data == nullptr)
	{
		std::cout << __FUNCTION__": Cant load HDR image: " + filepath << std::endl;
		return;
	}

	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	createImage.extent.width = width;
	createImage.extent.height = height;
	createImage.extent.depth = 1;
	createImage.mipLevels = 1;
	createImage.arrayLayers = 1;
	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createImage.format = VK_FORMAT_R32G32B32A32_SFLOAT;


	vkCreateImage(SmoothieCore::getDevice(), &createImage, nullptr, &image.image);

	VkBuffer stagingBuffer = nullptr;
	VmaAllocation stagingBufferAllocation = nullptr;

	VmaAllocationCreateInfo stagingBufferAllocInfo = {};
	stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	stagingBufferAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	stagingBufferAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	VkBufferCreateInfo stagingBufferCreateInfo{};
	stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferCreateInfo.size = width * height * 4 * sizeof(float);
	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	vmaCreateBuffer(VMA::getAllocator(), &stagingBufferCreateInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingBufferAllocation, nullptr);
	vmaCopyMemoryToAllocation(VMA::getAllocator(), data, stagingBufferAllocation, 0, width * height * 4 * sizeof(float));

	//Free data 
	if (data != nullptr)
	{
		stbi_image_free(data);
	}

	//Creating image
	VmaAllocationCreateInfo vmaImageAllocationInfo{};
	vmaImageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
	VmaAllocationInfo imageAllocationInfoDebug{};

	vmaCreateImage(VMA::getAllocator(), &createImage, &vmaImageAllocationInfo, &image.image, &image.allocation, &imageAllocationInfoDebug);

	transitionImageLayout(image.image, createImage.format, createImage.initialLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	copyBufferToImage(stagingBuffer, image.image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

	transitionImageLayout(image.image, createImage.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vmaDestroyBuffer(VMA::getAllocator(), stagingBuffer, stagingBufferAllocation);

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = image.image;
	imageViewInfo.format = createImage.format;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	vkCreateImageView(SmoothieCore::getDevice(), &imageViewInfo, nullptr, &image.imageView);
}

static void createCubemapImage(Image& image) 
{
	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	createImage.extent.width = 512;
	createImage.extent.height = 512;
	createImage.extent.depth = 1;
	createImage.mipLevels = 1;
	createImage.arrayLayers = 6;
	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createImage.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	createImage.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.priority = 1.0f;

	vmaCreateImage(VMA::getAllocator(), &createImage, &allocInfo, &image.image, &image.allocation, &image.allocationInfo);

	VkImageViewCreateInfo createImageView{};
	createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createImageView.image = image.image;
	createImageView.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	createImageView.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	createImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createImageView.subresourceRange.baseMipLevel = 0;
	createImageView.subresourceRange.levelCount = 1;
	createImageView.subresourceRange.baseArrayLayer = 0;
	createImageView.subresourceRange.layerCount = 6;
	vkCreateImageView(SmoothieCore::getDevice(), &createImageView, nullptr, &image.imageView);
}

struct DescriptorSetHelper 
{
	VkDescriptorSet descriptorSet = nullptr;
	VkDescriptorSetLayout descriptorSetLayout = nullptr;
	VkDescriptorPool descriptorPool = nullptr;
};

static void createDescriptorSetsLayouts(DescriptorSetHelper& helper, Image& image)
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;
	vkCreateDescriptorPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &helper.descriptorPool);


	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;
	vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &layoutInfo, nullptr, &helper.descriptorSetLayout);


	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = helper.descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &helper.descriptorSetLayout;
	vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &helper.descriptorSet);

	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgInfo.imageView = image.imageView;
	imgInfo.sampler = Samplers::Texture2DModelSampler;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = helper.descriptorSet;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pImageInfo = &imgInfo;

	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
}

static void destroyDescriptorSetsLayouts(DescriptorSetHelper& helper) 
{
	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), helper.descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(SmoothieCore::getDevice(), helper.descriptorPool, nullptr);
}

struct CubemapPipelineHelper
{
	VkShaderModule vertexShader = nullptr;
	VkShaderModule fragmentShader = nullptr;
	VkDescriptorSetLayout descriptorSetLayout = nullptr;
	VkRenderPass renderPass = nullptr;
};

static void createGraphicsPipelineCubemap(
	VkPipeline& pipeline, 
	VkPipelineLayout& pipelineLayout, 
	CubemapPipelineHelper& helper)
{
	VkPushConstantRange range = {};
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	range.offset = 0;
	range.size = 2 * sizeof(Matrix4x4);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &helper.descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &range;

	vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

	//Pipeline creation
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;


	//Shader stages
	VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
	vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderPipelineCreateInfo.pName = "main";
	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderPipelineCreateInfo.module = helper.vertexShader;

	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderPipelineCreateInfo.pName = "main";
	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderPipelineCreateInfo.module = helper.fragmentShader;

	VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;

	pipelineInfo.pVertexInputState = &PostProcessing::standardVertexState;
	pipelineInfo.pInputAssemblyState = &PostProcessing::standardAssemblyState;
	pipelineInfo.pViewportState = &PostProcessing::viewportState;
	pipelineInfo.pRasterizationState = &PostProcessing::rasterizer;
	pipelineInfo.pDepthStencilState = &PostProcessing::pipelineDepthStencil;
	pipelineInfo.pMultisampleState = &PostProcessing::multisampling;
	pipelineInfo.pColorBlendState = &PostProcessing::colorBlending;
	pipelineInfo.pDynamicState = &PostProcessing::dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = helper.renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;

	vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
}

static void generateViewMatrices(std::array<SmoothieMath::Matrix4x4, 6>& matrices)
{
	SmoothieMath::Matrix4x4 matrix;
	
	matrix.lookAtMatrix({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
	matrices[0] = matrix;

	matrix.lookAtMatrix({ 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
	matrices[1] = matrix;

	matrix.lookAtMatrix({ 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });
	matrices[3] = matrix;

	matrix.lookAtMatrix({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
	matrices[2] = matrix;

	matrix.lookAtMatrix({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f });
	matrices[4] = matrix;

	matrix.lookAtMatrix({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f });
	matrices[5] = matrix;
}

struct PushContantHelper 
{
	SmoothieMath::Matrix4x4 projection;
	SmoothieMath::Matrix4x4 view;
};

static void generateHDRCubemap(
	VkPipeline pipeline,
	VkPipelineLayout pipelineLayout,
	VkRenderPass renderPass,
	VkFramebuffer framebuffer,
	VkDescriptorSet descriptorSet,
	const SmoothieMath::Matrix4x4& projectionMatrix,
	const std::array<SmoothieMath::Matrix4x4, 6>& viewMatrices,
	const Image& cubemapImage,
	const Image& renderTarget)
{
	auto commandBuffer = beginSingleTimeCommands();

	//Makes cubemap ready for transfering data to it
	VkImageMemoryBarrier cubemapTransferDST{};
	cubemapTransferDST.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	cubemapTransferDST.srcAccessMask = 0;
	cubemapTransferDST.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	cubemapTransferDST.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	cubemapTransferDST.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	cubemapTransferDST.image = cubemapImage.image;
	cubemapTransferDST.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	cubemapTransferDST.subresourceRange.baseMipLevel = 0;
	cubemapTransferDST.subresourceRange.levelCount = 1;
	cubemapTransferDST.subresourceRange.baseArrayLayer = 0;
	cubemapTransferDST.subresourceRange.layerCount = 6;
	vkCmdPipelineBarrier(commandBuffer.buffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_DEPENDENCY_BY_REGION_BIT,
		0, nullptr,
		0, nullptr,
		1, &cubemapTransferDST);


	for (int i = 0; i < 6; i++) 
	{
		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(512);
		viewport.height = static_cast<float>(512);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRenderPassBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		beginInfo.framebuffer = framebuffer;
		beginInfo.renderPass = renderPass;

		VkClearValue clearColor{};
		clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };

		beginInfo.clearValueCount = 1;
		beginInfo.pClearValues = &clearColor;

		beginInfo.renderArea.extent = { 512, 512 };
		beginInfo.renderArea.offset = { 0, 0 };

		vkCmdSetViewport(commandBuffer.buffer, 0, 1, &viewport);
		vkCmdBeginRenderPass(commandBuffer.buffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdSetViewport(commandBuffer.buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { 512, 512 };
		vkCmdSetScissor(commandBuffer.buffer, 0, 1, &scissor);

		vkCmdBindDescriptorSets(commandBuffer.buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet,
			0, nullptr);

		PushContantHelper pushConstants;
		pushConstants.projection = projectionMatrix;
		pushConstants.view = viewMatrices[i];

		vkCmdPushConstants(commandBuffer.buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushContantHelper), &pushConstants);

		vkCmdDraw(commandBuffer.buffer, 36, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffer.buffer);
		
		//Makes target image ready for copying
		VkImageMemoryBarrier targetTransition{};
		targetTransition.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		targetTransition.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		targetTransition.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		targetTransition.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		targetTransition.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		targetTransition.image = renderTarget.image;
		targetTransition.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		targetTransition.subresourceRange.baseMipLevel = 0;
		targetTransition.subresourceRange.levelCount = 1;
		targetTransition.subresourceRange.baseArrayLayer = 0;
		targetTransition.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(commandBuffer.buffer,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
			0, nullptr,
			0, nullptr,
			1, &targetTransition);

		//Copies data from target image to a cubemap layer
		VkImageCopy copyRegions{};
		copyRegions.extent = { 512, 512, 1 };
		copyRegions.srcOffset = copyRegions.dstOffset = { 0, 0, 0 };
		copyRegions.srcSubresource.baseArrayLayer = 0;
		copyRegions.srcSubresource.layerCount = 1;
		copyRegions.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegions.dstSubresource.baseArrayLayer = i;
		copyRegions.dstSubresource.layerCount = 1;
		copyRegions.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		vkCmdCopyImage(commandBuffer.buffer,
			renderTarget.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			cubemapImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &copyRegions);

		//Transition target back into attachment optimal
		targetTransition.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		targetTransition.dstAccessMask = 0;
		targetTransition.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		targetTransition.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		targetTransition.image = renderTarget.image;
		targetTransition.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		targetTransition.subresourceRange.baseMipLevel = 0;
		targetTransition.subresourceRange.levelCount = 1;
		targetTransition.subresourceRange.baseArrayLayer = 0;
		targetTransition.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(commandBuffer.buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
			0, nullptr,
			0, nullptr,
			1, &targetTransition);


	}
	
	//Makes cubemap ready for sampling in shader
	VkImageMemoryBarrier imageMemoryBarier{};
	imageMemoryBarier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarier.image = cubemapImage.image;
	imageMemoryBarier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarier.subresourceRange.levelCount = 1;
	imageMemoryBarier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarier.subresourceRange.layerCount = 6;
	vkCmdPipelineBarrier(commandBuffer.buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_DEPENDENCY_BY_REGION_BIT,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarier);

	endSingleTimeCommands(commandBuffer);
}

static void createTempRenderTarget(Image& image, unsigned int width, unsigned int height) 
{
	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	createImage.extent = { width , height , 1 };
	createImage.mipLevels = 1;
	createImage.arrayLayers = 1;
	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createImage.format = VK_FORMAT_R32G32B32A32_SFLOAT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.priority = 1.0f;

	vmaCreateImage(VMA::getAllocator(), &createImage, &allocInfo, &image.image, &image.allocation, &image.allocationInfo);

	VkImageViewCreateInfo createImageView{};
	createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createImageView.image = image.image;
	createImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createImageView.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	createImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createImageView.subresourceRange.levelCount = 1;
	createImageView.subresourceRange.layerCount = 1;

	vkCreateImageView(SmoothieCore::getDevice(), &createImageView, nullptr, &image.imageView);
}

static void createIrradianceMap(Image& image) 
{
	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	createImage.extent.width = 32;
	createImage.extent.height = 32;
	createImage.extent.depth = 1;
	createImage.mipLevels = 1;
	createImage.arrayLayers = 6;
	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createImage.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	createImage.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.priority = 1.0f;

	vmaCreateImage(VMA::getAllocator(), &createImage, &allocInfo, &image.image, &image.allocation, &image.allocationInfo);

	VkImageViewCreateInfo createImageView{};
	createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createImageView.image = image.image;
	createImageView.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	createImageView.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	createImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createImageView.subresourceRange.baseMipLevel = 0;
	createImageView.subresourceRange.levelCount = 1;
	createImageView.subresourceRange.baseArrayLayer = 0;
	createImageView.subresourceRange.layerCount = 6;
	vkCreateImageView(SmoothieCore::getDevice(), &createImageView, nullptr, &image.imageView);
}

static void generateIrradianceCubemap(
	VkPipeline pipeline,
	VkPipelineLayout pipelineLayout,
	VkRenderPass renderPass,
	VkFramebuffer framebuffer,
	VkDescriptorSet descriptorSet,
	const SmoothieMath::Matrix4x4& projectionMatrix,
	const std::array<SmoothieMath::Matrix4x4, 6>& viewMatrices,
	const Image& irradianceImage,
	const Image& renderTarget) 
{
	auto commandBuffer = beginSingleTimeCommands();

	//Makes cubemap ready for transfering data to it
	VkImageMemoryBarrier cubemapTransferDST{};
	cubemapTransferDST.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	cubemapTransferDST.srcAccessMask = 0;
	cubemapTransferDST.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	cubemapTransferDST.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	cubemapTransferDST.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	cubemapTransferDST.image = irradianceImage.image;
	cubemapTransferDST.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	cubemapTransferDST.subresourceRange.baseMipLevel = 0;
	cubemapTransferDST.subresourceRange.levelCount = 1;
	cubemapTransferDST.subresourceRange.baseArrayLayer = 0;
	cubemapTransferDST.subresourceRange.layerCount = 6;
	vkCmdPipelineBarrier(commandBuffer.buffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_DEPENDENCY_BY_REGION_BIT,
		0, nullptr,
		0, nullptr,
		1, &cubemapTransferDST);


	for (int i = 0; i < 6; i++)
	{
		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(32);
		viewport.height = static_cast<float>(32);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRenderPassBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		beginInfo.framebuffer = framebuffer;
		beginInfo.renderPass = renderPass;

		VkClearValue clearColor{};
		clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };

		beginInfo.clearValueCount = 1;
		beginInfo.pClearValues = &clearColor;

		beginInfo.renderArea.extent = { 32, 32 };
		beginInfo.renderArea.offset = { 0, 0 };

		vkCmdSetViewport(commandBuffer.buffer, 0, 1, &viewport);
		vkCmdBeginRenderPass(commandBuffer.buffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdSetViewport(commandBuffer.buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { 32, 32 };
		vkCmdSetScissor(commandBuffer.buffer, 0, 1, &scissor);

		vkCmdBindDescriptorSets(commandBuffer.buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet,
			0, nullptr);

		PushContantHelper pushConstants;
		pushConstants.projection = projectionMatrix;
		pushConstants.view = viewMatrices[i];

		vkCmdPushConstants(commandBuffer.buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushContantHelper), &pushConstants);

		vkCmdDraw(commandBuffer.buffer, 36, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffer.buffer);

		//Makes target image ready for copying
		VkImageMemoryBarrier targetTransition{};
		targetTransition.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		targetTransition.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		targetTransition.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		targetTransition.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		targetTransition.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		targetTransition.image = renderTarget.image;
		targetTransition.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		targetTransition.subresourceRange.baseMipLevel = 0;
		targetTransition.subresourceRange.levelCount = 1;
		targetTransition.subresourceRange.baseArrayLayer = 0;
		targetTransition.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(commandBuffer.buffer,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
			0, nullptr,
			0, nullptr,
			1, &targetTransition);

		//Copies data from target image to a cubemap layer
		VkImageCopy copyRegions{};
		copyRegions.extent = { 32, 32, 1 };
		copyRegions.srcOffset = copyRegions.dstOffset = { 0, 0, 0 };
		copyRegions.srcSubresource.baseArrayLayer = 0;
		copyRegions.srcSubresource.layerCount = 1;
		copyRegions.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegions.dstSubresource.baseArrayLayer = i;
		copyRegions.dstSubresource.layerCount = 1;
		copyRegions.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		vkCmdCopyImage(commandBuffer.buffer,
			renderTarget.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			irradianceImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &copyRegions);


		//Transition target back into attachment optimal
		targetTransition.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		targetTransition.dstAccessMask = 0;
		targetTransition.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		targetTransition.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		targetTransition.image = renderTarget.image;
		targetTransition.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		targetTransition.subresourceRange.baseMipLevel = 0;
		targetTransition.subresourceRange.levelCount = 1;
		targetTransition.subresourceRange.baseArrayLayer = 0;
		targetTransition.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(commandBuffer.buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
			0, nullptr,
			0, nullptr,
			1, &targetTransition);
	}

	//Makes cubemap ready for sampling in shader
	VkImageMemoryBarrier imageMemoryBarier{};
	imageMemoryBarier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarier.image = irradianceImage.image;
	imageMemoryBarier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarier.subresourceRange.levelCount = 1;
	imageMemoryBarier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarier.subresourceRange.layerCount = 6;
	vkCmdPipelineBarrier(commandBuffer.buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_DEPENDENCY_BY_REGION_BIT,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarier);

	endSingleTimeCommands(commandBuffer);
}

Image PBRCubemaps::HDRCubemap;
Image PBRCubemaps::IrradianceMap;
Image PBRCubemaps::PrefilterMap;

static void createPrefilterMap(Image& image) 
{
	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	createImage.extent.width = 128;
	createImage.extent.height = 128;
	createImage.extent.depth = 1;
	createImage.mipLevels = 5;
	createImage.arrayLayers = 6;
	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createImage.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	createImage.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.priority = 1.0f;

	vmaCreateImage(VMA::getAllocator(), &createImage, &allocInfo, &image.image, &image.allocation, &image.allocationInfo);

	VkImageViewCreateInfo createImageView{};
	createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createImageView.image = image.image;
	createImageView.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	createImageView.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	createImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createImageView.subresourceRange.baseMipLevel = 0;
	createImageView.subresourceRange.levelCount = 5;
	createImageView.subresourceRange.baseArrayLayer = 0;
	createImageView.subresourceRange.layerCount = 6;
	vkCreateImageView(SmoothieCore::getDevice(), &createImageView, nullptr, &image.imageView);
}

static void createPrefilteredTargetMipChain(std::array<Image, 5>& tempImageMipChain)
{
	for (int mip = 0; mip < tempImageMipChain.size(); mip++)
	{
		const unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		createTempRenderTarget(tempImageMipChain[mip], mipWidth, mipWidth);
	}
}

static void destroyPrefilteredTargetMipChain(std::array<Image, 5>& tempImageMipChain)
{
	for (int mip = 0; mip < tempImageMipChain.size(); mip++)
	{
		tempImageMipChain[mip].destroyImage();
	}
}

static void createFramebufferChain(
	std::array<VkFramebuffer, 5>& framebufferMipChain,
	const std::array<Image, 5>& tempImageMipChain,
	VkRenderPass renderpass)
{
	for (int mip = 0; mip < tempImageMipChain.size(); mip++)
	{
		const unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		createFramebuffer(renderpass, tempImageMipChain[mip].imageView, framebufferMipChain[mip], mipWidth, mipWidth);
	}
}

static void destroyFrambufferChain(std::array<VkFramebuffer, 5>& framebufferMipChain)
{
	for (int mip = 0; mip < framebufferMipChain.size(); mip++)
	{
		vkDestroyFramebuffer(SmoothieCore::getDevice(), framebufferMipChain[mip], nullptr);
	}
}

static void createGraphicsPipelinePrefilter(VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, CubemapPipelineHelper& helper)
{
	VkPushConstantRange range{};
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	range.offset = 0;
	range.size = 2 * sizeof(Matrix4x4) + sizeof(float);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &helper.descriptorSetLayout;

	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &range;

	vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

	//Pipeline creation
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;


	//Shader stages
	VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
	vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderPipelineCreateInfo.pName = "main";
	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderPipelineCreateInfo.module = helper.vertexShader;

	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderPipelineCreateInfo.pName = "main";
	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderPipelineCreateInfo.module = helper.fragmentShader;

	VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;

	pipelineInfo.pVertexInputState = &PostProcessing::standardVertexState;
	pipelineInfo.pInputAssemblyState = &PostProcessing::standardAssemblyState;
	pipelineInfo.pViewportState = &PostProcessing::viewportState;
	pipelineInfo.pRasterizationState = &PostProcessing::rasterizer;
	pipelineInfo.pDepthStencilState = &PostProcessing::pipelineDepthStencil;
	pipelineInfo.pMultisampleState = &PostProcessing::multisampling;
	pipelineInfo.pColorBlendState = &PostProcessing::colorBlending;
	pipelineInfo.pDynamicState = &PostProcessing::dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = helper.renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;

	vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

}

struct PrefilterPushConstantHelper 
{
	SmoothieMath::Matrix4x4 projection;
	SmoothieMath::Matrix4x4 view;
	float roughness = 0;
};

static void drawPrefilterImageData(
	VkCommandBuffer commandBuffer,
	const std::array<VkFramebuffer, 5>& framebuffers,
	VkRenderPass renderPass,
	VkPipeline pipeline,
	VkDescriptorSet descriptorSet,
	VkPipelineLayout pipelineLayout,
	const SmoothieMath::Matrix4x4& projectionMatrix,
	const std::array<SmoothieMath::Matrix4x4, 6> viewMatrices,
	int layer,
	const std::array<Image, 5>& renderTargets,
	const Image& prefilterImage

)
{

	for (unsigned int mip = 0; mip < framebuffers.size(); mip++)
	{
		unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(mipWidth);
		viewport.height = static_cast<float>(mipWidth);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRenderPassBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		beginInfo.framebuffer = framebuffers[mip];
		beginInfo.renderPass = renderPass;

		VkClearValue clearColor{};
		clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };

		beginInfo.clearValueCount = 1;
		beginInfo.pClearValues = &clearColor;

		beginInfo.renderArea.extent = { mipWidth, mipWidth };
		beginInfo.renderArea.offset = { 0, 0 };

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { mipWidth, mipWidth };
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet,
			0, nullptr);


		PrefilterPushConstantHelper pushConstants;
		pushConstants.projection = projectionMatrix;
		pushConstants.view = viewMatrices[layer];
		pushConstants.roughness = (float)mip / (float)(framebuffers.size() - 1);

		vkCmdPushConstants(commandBuffer, pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
			sizeof(PrefilterPushConstantHelper), &pushConstants);



		vkCmdDraw(commandBuffer, 36, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffer);

		//Makes target image ready for copying
		VkImageMemoryBarrier targetTransition{};
		targetTransition.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		targetTransition.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		targetTransition.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		targetTransition.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		targetTransition.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		targetTransition.image = renderTargets[mip].image;
		targetTransition.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		targetTransition.subresourceRange.baseMipLevel = 0;
		targetTransition.subresourceRange.levelCount = 1;
		targetTransition.subresourceRange.baseArrayLayer = 0;
		targetTransition.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
			0, nullptr,
			0, nullptr,
			1, &targetTransition);

		//Copies data from target image to a cubemap layer
		VkImageCopy copyRegions{};
		copyRegions.extent = { mipWidth, mipWidth, 1 };
		copyRegions.srcOffset = copyRegions.dstOffset = { 0, 0, 0 };
		copyRegions.srcSubresource.baseArrayLayer = 0;
		copyRegions.srcSubresource.layerCount = 1;
		copyRegions.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegions.srcSubresource.mipLevel = 0;
		copyRegions.dstSubresource.baseArrayLayer = layer;
		copyRegions.dstSubresource.layerCount = 1;
		copyRegions.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegions.dstSubresource.mipLevel = mip;
		vkCmdCopyImage(commandBuffer,
			renderTargets[mip].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			prefilterImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &copyRegions);

		//Transition target back into attachment optimal
		targetTransition.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		targetTransition.dstAccessMask = 0;
		targetTransition.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		targetTransition.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		targetTransition.image = renderTargets[mip].image;
		targetTransition.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		targetTransition.subresourceRange.baseMipLevel = 0;
		targetTransition.subresourceRange.levelCount = 1;
		targetTransition.subresourceRange.baseArrayLayer = 0;
		targetTransition.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
			0, nullptr,
			0, nullptr,
			1, &targetTransition);
	}

}

static void generatePrefilterCubemap(
	VkPipeline pipeline,
	VkPipelineLayout pipelineLayout,
	VkRenderPass renderPass,
	std::array<VkFramebuffer, 5> framebuffers,
	VkDescriptorSet descriptorSet,
	const SmoothieMath::Matrix4x4& projectionMatrix,
	const std::array<SmoothieMath::Matrix4x4, 6>& viewMatrices,
	const Image& prefilterImage,
	const std::array<Image, 5>& renderTargets)
{
	auto commandBuffer = beginSingleTimeCommands();

	//Makes cubemap ready for transfering data to it
	VkImageMemoryBarrier cubemapTransferDST{};
	cubemapTransferDST.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	cubemapTransferDST.srcAccessMask = 0;
	cubemapTransferDST.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	cubemapTransferDST.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	cubemapTransferDST.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	cubemapTransferDST.image = prefilterImage.image;
	cubemapTransferDST.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	cubemapTransferDST.subresourceRange.baseMipLevel = 0;
	cubemapTransferDST.subresourceRange.levelCount = 5;
	cubemapTransferDST.subresourceRange.baseArrayLayer = 0;
	cubemapTransferDST.subresourceRange.layerCount = 6;
	vkCmdPipelineBarrier(commandBuffer.buffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_DEPENDENCY_BY_REGION_BIT,
		0, nullptr,
		0, nullptr,
		1, &cubemapTransferDST);


	for (int layer = 0; layer < 6; layer++)
	{
		drawPrefilterImageData(commandBuffer.buffer, framebuffers, renderPass, pipeline, descriptorSet, pipelineLayout, projectionMatrix,
			viewMatrices, layer, renderTargets, prefilterImage);
	}

	//Makes cubemap ready for sampling in shader
	VkImageMemoryBarrier imageMemoryBarier{};
	imageMemoryBarier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarier.image = prefilterImage.image;
	imageMemoryBarier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarier.subresourceRange.levelCount = 5;
	imageMemoryBarier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarier.subresourceRange.layerCount = 6;
	vkCmdPipelineBarrier(commandBuffer.buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_DEPENDENCY_BY_REGION_BIT,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarier);

	endSingleTimeCommands(commandBuffer);
}

void PBRCubemaps::create(const std::string& file)
{
	//****************************************** .hdr image -> hdr cubemap conversion *****************************//
	
	//Loaging .hdr image into GPU
	Image hdriImage;
	getHdriImage(file, hdriImage);

	//Creating HDR Cubemap image
	createCubemapImage(HDRCubemap);

	//Temp render pass, target and framebuffer from that target
	VkRenderPass renderPass;
	createSinglePassRenderPass(renderPass, VK_FORMAT_R32G32B32A32_SFLOAT);
	VkFramebuffer framebuffer = nullptr;
	Image tempRenderTarget;
	createTempRenderTarget(tempRenderTarget, 512, 512);
	createFramebuffer(renderPass, tempRenderTarget.imageView, framebuffer, 512, 512);
	
	//Descriptors used in shader
	DescriptorSetHelper descriptors;
	createDescriptorSetsLayouts(descriptors, hdriImage);

	//Pipeline creation
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
	CubemapPipelineHelper pipelineHelper;
	pipelineHelper.vertexShader = PostProcessingShaders::cubemapVertex;
	pipelineHelper.fragmentShader = PostProcessingShaders::HDRToCubemap;
	pipelineHelper.renderPass = renderPass;
	pipelineHelper.descriptorSetLayout = descriptors.descriptorSetLayout;
	createGraphicsPipelineCubemap(pipeline, pipelineLayout, pipelineHelper);
	
	//Matrices used in shader
	std::array<SmoothieMath::Matrix4x4, 6> viewMatrices;
	generateViewMatrices(viewMatrices);
	SmoothieMath::Matrix4x4 projectionMatrix;
	projectionMatrix.perspectiveProjection(90.0f, 1.0f, 0.1f, 10.0f);
	

	generateHDRCubemap(pipeline, pipelineLayout, renderPass, framebuffer, descriptors.descriptorSet, projectionMatrix, viewMatrices, HDRCubemap, tempRenderTarget);


	//Freeing used resources
	destroyPipelineData(pipelineLayout, pipeline);
	destroyDescriptorSetsLayouts(descriptors);
	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr);
	framebuffer = nullptr;
	tempRenderTarget.destroyImage();
	vkDestroyRenderPass(SmoothieCore::getDevice(), renderPass, nullptr);
	renderPass = nullptr;
	hdriImage.destroyImage();
	
	//***********************************************************************************************************//
	//****************************************** hdr cubemap -> irradiance cubemap ******************************//
	

	createIrradianceMap(IrradianceMap);
	createTempRenderTarget(tempRenderTarget, 32, 32);
	createSinglePassRenderPass(renderPass, VK_FORMAT_R32G32B32A32_SFLOAT);
	createFramebuffer(renderPass, tempRenderTarget.imageView, framebuffer, 32, 32);
	createDescriptorSetsLayouts(descriptors, HDRCubemap);

	pipelineHelper.vertexShader = PostProcessingShaders::cubemapVertex;
	pipelineHelper.fragmentShader = PostProcessingShaders::Irradiance;
	pipelineHelper.renderPass = renderPass;
	pipelineHelper.descriptorSetLayout = descriptors.descriptorSetLayout;
	createGraphicsPipelineCubemap(pipeline, pipelineLayout, pipelineHelper);

	generateIrradianceCubemap(pipeline, pipelineLayout, renderPass, framebuffer, descriptors.descriptorSet,
		projectionMatrix, viewMatrices, IrradianceMap, tempRenderTarget);

	destroyPipelineData(pipelineLayout, pipeline);
	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr);
	framebuffer = nullptr;
	vkDestroyRenderPass(SmoothieCore::getDevice(), renderPass, nullptr);
	renderPass = nullptr;
	tempRenderTarget.destroyImage();

	//***********************************************************************************************************//
	//****************************************** hdr cubemap -> prefilter cubemap *******************************//

	createPrefilterMap(PrefilterMap);

	std::array<Image, 5> tempImageMipChain;
	createPrefilteredTargetMipChain(tempImageMipChain);
	createSinglePassRenderPass(renderPass, VK_FORMAT_R32G32B32A32_SFLOAT);
	
	std::array<VkFramebuffer, 5> framebufferMipChain;
	createFramebufferChain(framebufferMipChain, tempImageMipChain, renderPass);

	pipelineHelper.vertexShader = PostProcessingShaders::cubemapVertex;
	pipelineHelper.fragmentShader = PostProcessingShaders::Prefilter;
	pipelineHelper.renderPass = renderPass;
	pipelineHelper.descriptorSetLayout = descriptors.descriptorSetLayout;
	createGraphicsPipelinePrefilter(pipeline, pipelineLayout, pipelineHelper);

	generatePrefilterCubemap(pipeline, pipelineLayout, renderPass, framebufferMipChain, descriptors.descriptorSet, projectionMatrix,
		viewMatrices, PrefilterMap, tempImageMipChain);

	destroyPipelineData(pipelineLayout, pipeline);
	destroyDescriptorSetsLayouts(descriptors);
	destroyFrambufferChain(framebufferMipChain);
	vkDestroyRenderPass(SmoothieCore::getDevice(), renderPass, nullptr);
	destroyPrefilteredTargetMipChain(tempImageMipChain);
}

void PBRCubemaps::destroy()
{
	PrefilterMap.destroyImage();
	IrradianceMap.destroyImage();
	HDRCubemap.destroyImage();
}
