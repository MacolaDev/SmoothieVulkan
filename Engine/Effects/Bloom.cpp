#include "Bloom.h"
#include "Core/RenderPass.h"
#include "Core/SmoothieCore.h"
#include "Core/Pipeline.h"
#include <iostream>

#include "Effects/Deferred_Core.h"

#include <cmath>

struct dwPushContant
{
	float width = 1;
	float height = 1;
	int mipLevel = 1;
	float padding = 0;
};

int Smoothie::DeferredRendering::Bloom::create()
{
	//********************************** Render pass **********************************//
	VkAttachmentDescription description{};
	description.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	description.samples = VK_SAMPLE_COUNT_1_BIT;
	description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = nullptr;
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &description;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = nullptr;
	if (vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		std::cout << "Failed to create render pass!" << std::endl;
		return 1;
	}

	//********************************** descriptor bindings **********************************//
	VkDescriptorSetLayoutBinding binding{};
	binding.binding = 0;
	binding.descriptorCount = 1;
	binding.pImmutableSamplers = nullptr;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &binding;
	if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor set layout!" << std::endl;
		return 1;
	}

	mipChainData.resize(5);
	VmaAllocationCreateInfo vmaImageAllocationInfo{};
	vmaImageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;

	//********************************** mipmap data **********************************//
	for (size_t i = 0; i < mipChainData.size(); i++)
	{
		auto& mip = mipChainData[i];
		const unsigned int w = mip.width =  static_cast<unsigned int>(SmoothieCore::getScrWidth() * std::pow(0.5f, i));
		const unsigned int h = mip.height = static_cast<unsigned int>(SmoothieCore::getScrHeight() * std::pow(0.5f, i));

		VkImageCreateInfo _mipImageCreateInfo{};
		_mipImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		_mipImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		_mipImageCreateInfo.extent.width = w;
		_mipImageCreateInfo.extent.height = h;
		_mipImageCreateInfo.extent.depth = 1;
		_mipImageCreateInfo.mipLevels = 1;
		_mipImageCreateInfo.arrayLayers = 1;
		_mipImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		_mipImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		_mipImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		_mipImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		_mipImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		_mipImageCreateInfo.format = description.format;
		if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_mipImageCreateInfo, &vmaImageAllocationInfo, &mip.image, &mip.allocation, nullptr) != VK_SUCCESS)
		{
			std::cout << "Failed to create mip chain image!" << std::endl;
			return 1;
		}

		VkImageViewCreateInfo _mipImageViewCreateInfo{};
		_mipImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		_mipImageViewCreateInfo.image = mip.image;
		_mipImageViewCreateInfo.format = _mipImageCreateInfo.format;
		_mipImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		_mipImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		_mipImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		_mipImageViewCreateInfo.subresourceRange.levelCount = 1;
		_mipImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		_mipImageViewCreateInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(SmoothieCore::getDevice(), &_mipImageViewCreateInfo, nullptr, &mip.imageView) != VK_SUCCESS)
		{
			std::cout << "Failed to create Image view for mip chain!" << std::endl;
			return 1;
		}

		VkFramebufferCreateInfo _MipFramebufferCreateInfo{};
		_MipFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		_MipFramebufferCreateInfo.renderPass = renderPass;
		_MipFramebufferCreateInfo.attachmentCount = 1;
		_MipFramebufferCreateInfo.pAttachments = &mip.imageView;
		_MipFramebufferCreateInfo.width = _mipImageCreateInfo.extent.width;
		_MipFramebufferCreateInfo.height = _mipImageCreateInfo.extent.height;
		_MipFramebufferCreateInfo.layers = 1;
		if (vkCreateFramebuffer(SmoothieCore::getDevice(), &_MipFramebufferCreateInfo, nullptr, &mip.framebuffer) != VK_SUCCESS)
		{
			std::cout << "Failed to create mip chain framebuffer!" << std::endl;
			return 1;
		}

		VkDescriptorPoolSize texturePoolSize{};
		texturePoolSize.type = binding.descriptorType;
		texturePoolSize.descriptorCount = 2;
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &texturePoolSize;
		poolInfo.maxSets = 2;
		if (vkCreateDescriptorPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &mip.descriptorPool) != VK_SUCCESS)
		{
			std::cout << "Failed to create mip chain descriptor pool!" << std::endl;
			return 1;
		}

		VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
		descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocInfo.descriptorPool = mip.descriptorPool;
		descriptorSetAllocInfo.descriptorSetCount = 1;
		descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
		if (VkResult result = vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &mip.dwnDescriptorSet); result != VK_SUCCESS)
		{
			std::cout << "Failed to allocate mip chain descriptor set!" << std::endl;
			return 1;
		}
		
		if (VkResult result = vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &mip.upDescriptorSet); result != VK_SUCCESS)
		{
			std::cout << "Failed to allocate mip chain descriptor set!" << std::endl;
			return 1;
		}

	}


	//********************************** Updating descriptors with data **********************************//

	//First descriptor gets a data from a higlight shader
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = HDRImageView;
	imageInfo.sampler = ClampToEdgeLINEAR;
	VkWriteDescriptorSet _WriteIrradinaceSet{};
	_WriteIrradinaceSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	_WriteIrradinaceSet.dstSet = mipChainData[0].dwnDescriptorSet;
	_WriteIrradinaceSet.dstArrayElement = 0;
	_WriteIrradinaceSet.descriptorType = binding.descriptorType;
	_WriteIrradinaceSet.descriptorCount = 1;
	_WriteIrradinaceSet.pBufferInfo = nullptr;
	_WriteIrradinaceSet.pImageInfo = &imageInfo;
	_WriteIrradinaceSet.dstBinding = 0;
	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_WriteIrradinaceSet, 0, nullptr);

	//Next descriptor set has an image from the set from the before
	for (size_t i = 1; i < mipChainData.size() - 1; i++)
	{
		imageInfo.imageView = mipChainData[i - 1].imageView;
		_WriteIrradinaceSet.pImageInfo = &imageInfo;
		_WriteIrradinaceSet.dstSet = mipChainData[i].dwnDescriptorSet;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_WriteIrradinaceSet, 0, nullptr);
	}

	//last image poits to
	imageInfo.imageView = mipChainData[mipChainData.size() - 2].imageView;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	_WriteIrradinaceSet.pImageInfo = &imageInfo;
	_WriteIrradinaceSet.dstSet = mipChainData.back().dwnDescriptorSet;
	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_WriteIrradinaceSet, 0, nullptr);


	for (size_t i = mipChainData.size() - 1; i > 0; i--)
	{
		const auto& mip = mipChainData[i];
		const auto& mip_before = mipChainData[i-1];
		
		imageInfo.imageView = mip.imageView;
		_WriteIrradinaceSet.pImageInfo = &imageInfo;
		_WriteIrradinaceSet.dstSet = mip_before.upDescriptorSet;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_WriteIrradinaceSet, 0, nullptr);
	}


	//********************************** higlight pipeline **********************************//
	VkPipelineLayoutCreateInfo higlightPipelineLayoutInfo{};
	higlightPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	higlightPipelineLayoutInfo.setLayoutCount = 1;
	higlightPipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &higlightPipelineLayoutInfo, nullptr, &higlightPipelineLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create higlight pipeline layout!" << std::endl;
		return 1;
	}

	VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
	vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderPipelineCreateInfo.pName = "vertex_QUAD";
	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderPipelineCreateInfo.module = vertexShader;

	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderPipelineCreateInfo.pName = "fragment_HIGLIGHTS";
	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderPipelineCreateInfo.module = m_FragmentShader;


	VkGraphicsPipelineCreateInfo _HiglightsPipelineInfo{};
	_HiglightsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
	_HiglightsPipelineInfo.stageCount = 2;
	_HiglightsPipelineInfo.pStages = stages;
	Smoothie::DefaultPipelineState __state;
	__state.populate_pipeline(_HiglightsPipelineInfo);
	_HiglightsPipelineInfo.renderPass = renderPass;
	_HiglightsPipelineInfo.subpass = 0;
	_HiglightsPipelineInfo.basePipelineHandle = nullptr;
	_HiglightsPipelineInfo.layout = higlightPipelineLayout;
	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &_HiglightsPipelineInfo, nullptr, &higlightPipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create higlight pipeline!" << std::endl;
		return 1;
	}

	//********************************** downsampling pipeline **********************************//
	VkPipelineLayoutCreateInfo downPipelineLayoutInfo{};
	downPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	downPipelineLayoutInfo.setLayoutCount = 1;
	downPipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	VkPushConstantRange range{};
	range.offset = 0;
	range.size = sizeof(dwPushContant);
	range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	downPipelineLayoutInfo.pushConstantRangeCount = 1;
	downPipelineLayoutInfo.pPushConstantRanges = &range;
	if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &downPipelineLayoutInfo, nullptr, &downsamplingPipelineLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create downsampling pipeline layout!" << std::endl;
		return 1;
	}

	fragmentShaderPipelineCreateInfo.pName = "fragment_DOWNSAMPLE";
		
	VkGraphicsPipelineCreateInfo _DownsamplePipelineInfo = _HiglightsPipelineInfo;
	_DownsamplePipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	stages[1] = fragmentShaderPipelineCreateInfo;
	_DownsamplePipelineInfo.stageCount = 2;
	_DownsamplePipelineInfo.pStages = stages;
	_DownsamplePipelineInfo.renderPass = renderPass;
	_DownsamplePipelineInfo.subpass = 0;
	_DownsamplePipelineInfo.basePipelineHandle = nullptr;
	_DownsamplePipelineInfo.layout = downsamplingPipelineLayout;
	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &_DownsamplePipelineInfo, nullptr, &downsamplingPipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create downsampling pipeline!" << std::endl;
		return 1;
	}

	//********************************** upsampling pipeline **********************************//
	VkPipelineLayoutCreateInfo upPipelineLayoutInfo{};
	upPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	upPipelineLayoutInfo.setLayoutCount = 1;
	upPipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	range.offset = 0;
	range.size = sizeof(float);
	range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	upPipelineLayoutInfo.pushConstantRangeCount = 1;
	upPipelineLayoutInfo.pPushConstantRanges = &range;
	if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &downPipelineLayoutInfo, nullptr, &upsamplingPipelineLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create downsampling pipeline layout!" << std::endl;
		return 1;
	}

	fragmentShaderPipelineCreateInfo.pName = "fragment_UPSAMPLE";

	VkGraphicsPipelineCreateInfo _upsamplingPipelineInfo = _HiglightsPipelineInfo;
	_upsamplingPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	stages[1] = fragmentShaderPipelineCreateInfo;
	_upsamplingPipelineInfo.stageCount = 2;
	_upsamplingPipelineInfo.pStages = stages;
	_upsamplingPipelineInfo.renderPass = renderPass;
	_upsamplingPipelineInfo.subpass = 0;
	_upsamplingPipelineInfo.basePipelineHandle = nullptr;
	_upsamplingPipelineInfo.layout = upsamplingPipelineLayout;

	VkPipelineColorBlendAttachmentState ColorBlendAttachmentState{};
	ColorBlendAttachmentState.blendEnable = true;
	ColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	ColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	ColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
	VkPipelineColorBlendStateCreateInfo blending = __state.getColorBlendStateCreateInfo();
	blending.attachmentCount = 1;
	blending.pAttachments = &ColorBlendAttachmentState;
	blending.blendConstants[0] = 0.0f;
	blending.blendConstants[1] = 0.0f;
	blending.blendConstants[2] = 0.0f;
	blending.blendConstants[3] = 0.0f;
	_upsamplingPipelineInfo.pColorBlendState = &blending;
	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &_upsamplingPipelineInfo, nullptr, &upsamplingPipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create downsampling pipeline!" << std::endl;
		return 1;
	}


	return 0;
}

void Smoothie::DeferredRendering::Bloom::draw(VkCommandBuffer commandBuffer, VkDescriptorSet drawClassDescriptor, unsigned int ImageIndex) const
{
	//Get higlights
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, higlightPipeline);
	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.framebuffer = mipChainData[0].framebuffer;
	beginInfo.renderPass = renderPass;
	VkClearValue clearColor{};
	clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	beginInfo.clearValueCount = 1;
	beginInfo.pClearValues = &clearColor;
	beginInfo.renderArea.extent = { mipChainData[0].width, mipChainData[0].height};
	beginInfo.renderArea.offset = { 0, 0 };
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(mipChainData[0].width);
	viewport.height = static_cast<float>(mipChainData[0].height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { mipChainData[0].width, mipChainData[0].height };
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, higlightPipelineLayout, 0, 1, &mipChainData[0].dwnDescriptorSet, 0, nullptr);
	vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);

	VkMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		1, &memoryBarrier,
		0, nullptr,
		0, nullptr);

	//Down sample it
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, downsamplingPipeline);
	for (int i = 1; i < mipChainData.size(); i++)
	{
		const auto& _dat = mipChainData[i];
		beginInfo.framebuffer = _dat.framebuffer;
		beginInfo.renderArea.extent = { _dat.width, _dat.height };
		vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(_dat.width);
		viewport.height = static_cast<float>(_dat.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { _dat.width, _dat.height };
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		dwPushContant pushConstant;
		pushConstant.width = static_cast<float>(_dat.width);
		pushConstant.height = static_cast<float>(_dat.height);
		if (i == 1) 
		{
			pushConstant.mipLevel = 1;
		}
		else
		{
			pushConstant.mipLevel = 0;
		}
		
		vkCmdPushConstants(commandBuffer,  downsamplingPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(dwPushContant), &pushConstant);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, downsamplingPipelineLayout, 0, 1, &_dat.dwnDescriptorSet, 0, nullptr);
		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffer);

		VkMemoryBarrier memoryBarrier = {};
		memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		memoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			1, &memoryBarrier,
			0, nullptr,
			0, nullptr);
	}


	
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0,
		0, &memoryBarrier,
		0, nullptr,
		0, nullptr);


	//Upsample it
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, upsamplingPipeline);

	float filterRadius = 0.0025f;
	vkCmdPushConstants(commandBuffer, upsamplingPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &filterRadius);
	for (size_t i = mipChainData.size() - 2; i > 0; i--)
	{
		const auto& _dat = mipChainData[i];
		beginInfo.framebuffer = _dat.framebuffer;
		beginInfo.renderArea.extent = { _dat.width, _dat.height };
		vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(_dat.width);
		viewport.height = static_cast<float>(_dat.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { _dat.width, _dat.height };
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,upsamplingPipelineLayout, 0,1, &mipChainData[i].upDescriptorSet,0, nullptr);
		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffer);

		VkMemoryBarrier memoryBarrier = {};
		memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		memoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			1, &memoryBarrier,
			0, nullptr,
			0, nullptr);
	}
	int a = 0;
}

void Smoothie::DeferredRendering::Bloom::destroy()
{
	vkDestroyPipeline(SmoothieCore::getDevice(), upsamplingPipeline, nullptr), upsamplingPipeline = nullptr;
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), upsamplingPipelineLayout, nullptr), upsamplingPipelineLayout = nullptr;
	vkDestroyPipeline(SmoothieCore::getDevice(), downsamplingPipeline, nullptr), downsamplingPipeline = nullptr;
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), downsamplingPipelineLayout, nullptr), downsamplingPipelineLayout = nullptr;
	vkDestroyPipeline(SmoothieCore::getDevice(), higlightPipeline, nullptr), higlightPipeline = nullptr;
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), higlightPipelineLayout, nullptr), higlightPipelineLayout = nullptr;

	for (size_t i = 0; i < mipChainData.size(); i++)
	{
		auto& mip = mipChainData[i];
		vkDestroyDescriptorPool(SmoothieCore::getDevice(), mip.descriptorPool, nullptr);
		mip.descriptorPool = nullptr, mip.dwnDescriptorSet = nullptr, mip.upDescriptorSet;
		vkDestroyFramebuffer(SmoothieCore::getDevice(), mip.framebuffer, nullptr), mip.framebuffer = nullptr;
		vkDestroyImageView(SmoothieCore::getDevice(), mip.imageView, nullptr), mip.imageView = nullptr;
		vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), mip.image, mip.allocation);
		mip.image = nullptr, mip.allocation = nullptr;
	}
	
	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), descriptorSetLayout, nullptr), descriptorSetLayout = nullptr;
	vkDestroyRenderPass(SmoothieCore::getDevice(), renderPass, nullptr), renderPass = nullptr;
}

int Smoothie::DeferredRendering::Bloom::resize_callback()
{
	//Destroy last images and framebuffers
	for (size_t i = 0; i < mipChainData.size(); i++)
	{
		auto& mip = mipChainData[i];
		vkDestroyFramebuffer(SmoothieCore::getDevice(), mip.framebuffer, nullptr), mip.framebuffer = nullptr;
		vkDestroyImageView(SmoothieCore::getDevice(), mip.imageView, nullptr), mip.imageView = nullptr;
		vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), mip.image, mip.allocation);
		mip.image = nullptr, mip.allocation = nullptr;
	}

	VmaAllocationCreateInfo vmaImageAllocationInfo{};
	vmaImageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;

	//Create new ones with new sizes
	for (size_t i = 0; i < mipChainData.size(); i++)
	{
		auto& mip = mipChainData[i];
		const unsigned int w = mip.width = static_cast<unsigned int>(SmoothieCore::getScrWidth() * std::pow(0.5f, i));
		const unsigned int h = mip.height = static_cast<unsigned int>(SmoothieCore::getScrHeight() * std::pow(0.5f, i));

		VkImageCreateInfo _mipImageCreateInfo{};
		_mipImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		_mipImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		_mipImageCreateInfo.extent.width = w;
		_mipImageCreateInfo.extent.height = h;
		_mipImageCreateInfo.extent.depth = 1;
		_mipImageCreateInfo.mipLevels = 1;
		_mipImageCreateInfo.arrayLayers = 1;
		_mipImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		_mipImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		_mipImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		_mipImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		_mipImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		_mipImageCreateInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
		if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_mipImageCreateInfo, &vmaImageAllocationInfo, &mip.image, &mip.allocation, nullptr) != VK_SUCCESS)
		{
			std::cout << "Failed to create mip chain image!" << std::endl;
			return 1;
		}

		VkImageViewCreateInfo _mipImageViewCreateInfo{};
		_mipImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		_mipImageViewCreateInfo.image = mip.image;
		_mipImageViewCreateInfo.format = _mipImageCreateInfo.format;
		_mipImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		_mipImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		_mipImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		_mipImageViewCreateInfo.subresourceRange.levelCount = 1;
		_mipImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		_mipImageViewCreateInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(SmoothieCore::getDevice(), &_mipImageViewCreateInfo, nullptr, &mip.imageView) != VK_SUCCESS)
		{
			std::cout << "Failed to create Image view for mip chain!" << std::endl;
			return 1;
		}

		VkFramebufferCreateInfo _MipFramebufferCreateInfo{};
		_MipFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		_MipFramebufferCreateInfo.renderPass = renderPass;
		_MipFramebufferCreateInfo.attachmentCount = 1;
		_MipFramebufferCreateInfo.pAttachments = &mip.imageView;
		_MipFramebufferCreateInfo.width = _mipImageCreateInfo.extent.width;
		_MipFramebufferCreateInfo.height = _mipImageCreateInfo.extent.height;
		_MipFramebufferCreateInfo.layers = 1;
		if (vkCreateFramebuffer(SmoothieCore::getDevice(), &_MipFramebufferCreateInfo, nullptr, &mip.framebuffer) != VK_SUCCESS)
		{
			std::cout << "Failed to create mip chain framebuffer!" << std::endl;
			return 1;
		}
	}

	//Update descriptor sets
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = HDRImageView;
	imageInfo.sampler = ClampToEdgeLINEAR;
	VkWriteDescriptorSet _descriptorUpdate{};
	_descriptorUpdate.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	_descriptorUpdate.dstSet = mipChainData[0].dwnDescriptorSet;
	_descriptorUpdate.dstArrayElement = 0;
	_descriptorUpdate.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	_descriptorUpdate.descriptorCount = 1;
	_descriptorUpdate.pBufferInfo = nullptr;
	_descriptorUpdate.pImageInfo = &imageInfo;
	_descriptorUpdate.dstBinding = 0;
	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_descriptorUpdate, 0, nullptr);

	for (size_t i = 1; i < mipChainData.size() - 1; i++)
	{
		imageInfo.imageView = mipChainData[i - 1].imageView;
		_descriptorUpdate.pImageInfo = &imageInfo;
		_descriptorUpdate.dstSet = mipChainData[i].dwnDescriptorSet;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_descriptorUpdate, 0, nullptr);
	}
	imageInfo.imageView = mipChainData[mipChainData.size() - 2].imageView;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	_descriptorUpdate.pImageInfo = &imageInfo;
	_descriptorUpdate.dstSet = mipChainData.back().dwnDescriptorSet;
	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_descriptorUpdate, 0, nullptr);


	for (size_t i = mipChainData.size() - 1; i > 0; i--)
	{
		const auto& mip = mipChainData[i];
		const auto& mip_before = mipChainData[i - 1];

		imageInfo.imageView = mip.imageView;
		_descriptorUpdate.pImageInfo = &imageInfo;
		_descriptorUpdate.dstSet = mip_before.upDescriptorSet;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_descriptorUpdate, 0, nullptr);
	}

	return 0;
}

VkImageView Smoothie::DeferredRendering::Bloom::getFinalImage() const
{
	return mipChainData[1].imageView;
}
