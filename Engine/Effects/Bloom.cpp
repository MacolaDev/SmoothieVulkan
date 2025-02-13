#include "Bloom.h"
#define _SMOOTHIE_ENGINE
#include "Core/SmoothieCore.h"
#include "Core/RenderPass.h"
#include "Core/Shader.h"
#include "Core/PostProcessing.h"

static void createHiglightImage(Image& image, unsigned int width, unsigned int height) 
{
	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	createImage.extent = { width , height , 1 };
	createImage.mipLevels = 1;
	createImage.arrayLayers = 1;
	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createImage.format = VK_FORMAT_R16G16B16A16_SFLOAT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.priority = 1.0f;

	vmaCreateImage(VMA::getAllocator(), &createImage, &allocInfo, &image.image, &image.allocation, &image.allocationInfo);

	VkImageViewCreateInfo createImageView{};
	createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createImageView.image = image.image;
	createImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createImageView.format = createImage.format;
	createImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createImageView.subresourceRange.levelCount = 1;
	createImageView.subresourceRange.layerCount = 1;

	vkCreateImageView(SmoothieCore::getDevice(), &createImageView, nullptr, &image.imageView);
}

static void createFramebuffer(VkFramebuffer& framebuffer, unsigned int width, unsigned int height, const Image& image) 
{
	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = BloomPass::renderPass;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &image.imageView;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = 1;
	vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &framebuffer);
}

VkDescriptorPool Bloom::higlightDescriptorPool = nullptr;
VkDescriptorSetLayout Bloom::higlightDescriptorSetLayout = nullptr;
VkDescriptorSet Bloom::higlightDescriptorSet = nullptr;

static void createDescriptors( VkDescriptorPool& pool,  VkDescriptorSetLayout& layout, VkDescriptorSet& set, const Image& image) 
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;
	vkCreateDescriptorPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &pool);

	//Descriptor set layout
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = 0;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &layoutBinding;
	vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &layoutInfo, nullptr, &layout);

	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = pool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &layout;
	vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &set);

	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgInfo.imageView = image.imageView;
	imgInfo.sampler = Samplers::ClampToEdgeLINEAR;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = set;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pImageInfo = &imgInfo;
	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
}

static void destroyDescriptors( 
	VkDescriptorPool& descriptorPool, 
	VkDescriptorSetLayout& descriptorSetLayout,
	VkDescriptorSet& descriptorSet) 
{
	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), descriptorSetLayout, nullptr);
	descriptorSetLayout = nullptr;

	vkDestroyDescriptorPool(SmoothieCore::getDevice(), descriptorPool, nullptr);
	descriptorPool = nullptr;
	descriptorSet = nullptr;
}

VkPipeline Bloom::higlightPipeline = nullptr;
VkPipelineLayout Bloom::higlightPipelineLayout = nullptr;

VkPipeline Bloom::downsamplingPipeline = nullptr;
VkPipelineLayout Bloom::downsamplingPipelineLayout = nullptr;

static void createPipeline(
	VkPipeline& pipeline, 
	VkPipelineLayout& pipelineLayout, 
	VkDescriptorSetLayout& descriptorLayout, 
	VkShaderModule& fragmentShader, unsigned int pushConstantSize = 0) 
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorLayout;
	
	VkPushConstantRange range{};
	range.offset = 0;
	range.size = pushConstantSize;
	range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	if (pushConstantSize != 0) 
	{
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &range;
	}
	vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);


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
	fragmentShaderPipelineCreateInfo.module = fragmentShader;


	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	
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
	pipelineInfo.renderPass = BloomPass::renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;

	vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

}

static void destroyPipeline(VkPipeline& pipeline, VkPipelineLayout& layout) 
{
	vkDestroyPipeline(SmoothieCore::getDevice(), pipeline, nullptr);
	pipeline = nullptr;
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), layout, nullptr);
	layout = nullptr;
}

std::array<MipChainImage, numberOfMips> Bloom::mipChainImages;

struct dwPushContant 
{
	float width = 1; 
	float height = 1;
	int mipLevel = 1;
	float padding = 0;
};

static void createPipelineUpsampling(VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkDescriptorSetLayout& descriptorLayout) 
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorLayout;

	VkPushConstantRange range{};
	range.offset = 0;
	range.size = sizeof(float);
	range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &range;
	vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);


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
	fragmentShaderPipelineCreateInfo.module = PostProcessingShaders::upsampling;


	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	VkPipelineColorBlendAttachmentState state{};
	state.blendEnable = true;
	state.colorBlendOp = VK_BLEND_OP_ADD;
	state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
	VkPipelineColorBlendStateCreateInfo blending = PostProcessing::colorBlending;
	
	blending.attachmentCount = 1;
	blending.pAttachments = &state;
	blending.blendConstants[0] = 0.0f;
	blending.blendConstants[1] = 0.0f;
	blending.blendConstants[2] = 0.0f;
	blending.blendConstants[3] = 0.0f;

	VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;
	pipelineInfo.pVertexInputState = &PostProcessing::standardVertexState;
	pipelineInfo.pInputAssemblyState = &PostProcessing::standardAssemblyState;
	pipelineInfo.pViewportState = &PostProcessing::viewportState;
	pipelineInfo.pRasterizationState = &PostProcessing::rasterizer;
	pipelineInfo.pDepthStencilState = &PostProcessing::pipelineDepthStencil;
	pipelineInfo.pMultisampleState = &PostProcessing::multisampling;
	pipelineInfo.pColorBlendState = &blending;
	pipelineInfo.pDynamicState = &PostProcessing::dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = BloomPass::renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;

	vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
}

void Bloom::create(unsigned int width, unsigned int height, const Image& HDRIImage)
{
	//mipChainImages and framebuffers
	for (int mip = 0; mip < mipChainImages.size(); mip++)
	{
		const unsigned int w = static_cast<unsigned int>(width * std::pow(0.5, mip));
		const unsigned int h = static_cast<unsigned int>(height * std::pow(0.5, mip));
		mipChainImages[mip].width = w;
		mipChainImages[mip].height = h;
		createHiglightImage(mipChainImages[mip].mipImage, w, h);
		createFramebuffer(mipChainImages[mip].framebuffer, w, h, mipChainImages[mip].mipImage);
	}

	createDescriptors(higlightDescriptorPool, higlightDescriptorSetLayout, higlightDescriptorSet, HDRIImage);
	createPipeline(higlightPipeline, higlightPipelineLayout, higlightDescriptorSetLayout, PostProcessingShaders::higlights);

	//downsampling descriptor sets and pipelines
	for (int mip = 1; mip < mipChainImages.size(); mip++)
	{
		createDescriptors(
			mipChainImages[mip].dwDescriptorPool, 
			mipChainImages[mip].dwDescriptorSetLayout, 
			mipChainImages[mip].dwDescriptorSet, mipChainImages[mip - 1].mipImage);
		createPipeline(
			mipChainImages[mip].dwPipeline,
			mipChainImages[mip].dwPipelineLayout,
			mipChainImages[mip].dwDescriptorSetLayout,
			PostProcessingShaders::downsampling, static_cast<unsigned int>(sizeof(dwPushContant)));
	}

	//upsampling  
	for (size_t mip = mipChainImages.size() - 1; mip > 0; mip--)
	{
		createDescriptors(
			mipChainImages[mip-1].upDescriptorPool,
			mipChainImages[mip-1].upDescriptorSetLayout,
			mipChainImages[mip-1].upDescriptorSet, mipChainImages[mip].mipImage);
		createPipelineUpsampling(
			mipChainImages[mip-1].upPipeline,
			mipChainImages[mip-1].upPipelineLayout,
			mipChainImages[mip-1].upDescriptorSetLayout);
	}

}

static inline void translateToShaderReadOnly(VkCommandBuffer commandBuffer, const Image& image) 
{
	VkImageMemoryBarrier imageMemoryBarier{};
	imageMemoryBarier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarier.image = image.image;
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

static inline void translateToAttachment(VkCommandBuffer commandBuffer, const Image& image)
{
	VkImageMemoryBarrier imageMemoryBarier{};
	imageMemoryBarier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarier.image = image.image;
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

static inline void beginPass(VkCommandBuffer commandBuffer, unsigned int width, unsigned int height, VkFramebuffer framebuffer) 
{
	//Begin bass
	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.framebuffer = framebuffer;
	beginInfo.renderPass = BloomPass::renderPass;

	VkClearValue clearColor{};
	clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	beginInfo.clearValueCount = 1;
	beginInfo.pClearValues = &clearColor;
	beginInfo.renderArea.extent = { width, height };
	beginInfo.renderArea.offset = { 0, 0 };

	//Viewport data
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(width);
	viewport.height = static_cast<float>(height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Bloom::bindAndDraw(VkCommandBuffer commandBuffer)
{
	//Viewport data
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(1280);
	viewport.height = static_cast<float>(720);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	beginPass(commandBuffer, 1280, 720, mipChainImages[0].framebuffer);
	
	//Higligh image
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, higlightPipeline);
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { 1280, 720 };
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		higlightPipelineLayout, 0,
		1, &higlightDescriptorSet,
		0, nullptr);

	vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);

	//Downsampling
	translateToShaderReadOnly(commandBuffer, mipChainImages[0].mipImage);

	for (int i = 1; i < mipChainImages.size(); i++) 
	{
		beginPass(commandBuffer, mipChainImages[i].width, mipChainImages[i].height, mipChainImages[i].framebuffer);
		
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(mipChainImages[i].width);
		viewport.height = static_cast<float>(mipChainImages[i].height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { mipChainImages[i].width, mipChainImages[i].height };
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mipChainImages[i].dwPipeline);
		dwPushContant pushConstant;
		if (i == 1) 
		{
			pushConstant.mipLevel = 1;
		}
		else
		{
			pushConstant.mipLevel = 0;
		}
		pushConstant.width = static_cast<float>(mipChainImages[i].width);
		pushConstant.height = static_cast<float>(mipChainImages[i].height);
		
		vkCmdPushConstants(commandBuffer, 
			mipChainImages[i].dwPipelineLayout, 
			VK_SHADER_STAGE_FRAGMENT_BIT, 0, 
			sizeof(dwPushContant), &pushConstant);
		
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			mipChainImages[i].dwPipelineLayout, 0,
			1, &mipChainImages[i].dwDescriptorSet,
			0, nullptr);

		auto image = mipChainImages[i].mipImage.image;
		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
		
		vkCmdEndRenderPass(commandBuffer);

		//Transition attachment image to the one for sampling
		translateToShaderReadOnly(commandBuffer, mipChainImages[i].mipImage);

	}


	//upsampling
	for (size_t i = mipChainImages.size() - 1; i > 1; i--)
	{
		translateToAttachment(commandBuffer, mipChainImages[i-1].mipImage);
		beginPass(commandBuffer, mipChainImages[i-1].width, mipChainImages[i-1].height, mipChainImages[i-1].framebuffer);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(mipChainImages[i-1].width);
		viewport.height = static_cast<float>(mipChainImages[i-1].height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { mipChainImages[i-1].width, mipChainImages[i-1].height };
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mipChainImages[i-1].upPipeline);

		vkCmdPushConstants(commandBuffer,
			mipChainImages[i-1].upPipelineLayout,
			VK_SHADER_STAGE_FRAGMENT_BIT, 0,
			sizeof(float), &filterRadius);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			mipChainImages[i-1].upPipelineLayout, 0,
			1, &mipChainImages[i-1].upDescriptorSet,
			0, nullptr);

		vkCmdDraw(commandBuffer, 6, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);
		translateToShaderReadOnly(commandBuffer, mipChainImages[i - 1].mipImage);
	}

	//translateToAttachment(commandBuffer, mipChainImages[0].mipImage);

}

void Bloom::destroy()
{

	for (size_t mip = mipChainImages.size() - 1; mip > 0; mip--)
	{
		destroyPipeline(mipChainImages[mip].upPipeline,
			mipChainImages[mip].upPipelineLayout);
		destroyDescriptors(mipChainImages[mip].upDescriptorPool, mipChainImages[mip].upDescriptorSetLayout,
			mipChainImages[mip].upDescriptorSet);
	}

	for (size_t mip = 1; mip < mipChainImages.size(); mip++)
	{
		destroyPipeline(mipChainImages[mip].dwPipeline, mipChainImages[mip].dwPipelineLayout);
		destroyDescriptors(mipChainImages[mip].dwDescriptorPool, mipChainImages[mip].dwDescriptorSetLayout, 
			mipChainImages[mip].dwDescriptorSet);
	}

	destroyPipeline(higlightPipeline, higlightPipelineLayout);
	destroyDescriptors(higlightDescriptorPool, higlightDescriptorSetLayout, higlightDescriptorSet);
	for (size_t mip = 0; mip < mipChainImages.size(); mip++)
	{
		mipChainImages[mip].mipImage.destroyImage();
		vkDestroyFramebuffer(SmoothieCore::getDevice(), mipChainImages[mip].framebuffer, nullptr);
		mipChainImages[mip].framebuffer = nullptr;
	}

}

Image Bloom::getBloomImage()
{
	return mipChainImages[1].mipImage;
}
