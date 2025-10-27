#include "Deferred_Lighting.h"
#include "Core/SmoothieCore.h"
#include <array>
#include "Core/Pipeline.h"
#include "Core/Multithreading.h"
#include <cmath>
int Smoothie::DeferredRendering::Lighting_RenderPass::create()
{
	VkAttachmentDescription _attachmentDescription{};
	_attachmentDescription.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	_attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	_attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	_attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	_attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	_attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	_attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	VkAttachmentDescription attachments[1] =
	{
		_attachmentDescription
	};

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	if (vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS)
	{
		std::cout << "Failed to create render pass!" << std::endl;
		return 1;
	}

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = render_pass;
	createInfo.attachmentCount = 1;
	VkImageView _target_imgview = target.getImageView();
	createInfo.pAttachments = &_target_imgview;
	createInfo.width = SmoothieCore::getScrWidth();
	createInfo.height = SmoothieCore::getScrHeight();
	createInfo.layers = 1;
	if (vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &framebuffer) != VK_SUCCESS)
	{
		std::cout << "Failed to create framebuffer" << std::endl;
		return 1;
	}
    return 0;
}

int Smoothie::DeferredRendering::Lighting_RenderPass::resize_callback()
{
	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr);
	framebuffer = nullptr;
	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = render_pass;
	createInfo.attachmentCount = 1;
	VkImageView _target_imgview = target.getImageView();
	createInfo.pAttachments = &_target_imgview;
	createInfo.width = SmoothieCore::getScrWidth();
	createInfo.height = SmoothieCore::getScrHeight();
	createInfo.layers = 1;
	if (vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &framebuffer) != VK_SUCCESS)
	{
		std::cout << "Failed to create framebuffer" << std::endl;
		return 1;
	}

	return 0;
}

void Smoothie::DeferredRendering::Lighting_RenderPass::destroy()
{
	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr);
	framebuffer = nullptr;

	vkDestroyRenderPass(SmoothieCore::getDevice(), render_pass, nullptr);
	render_pass = nullptr;
}

void Smoothie::DeferredRendering::Lighting_RenderPass::bindPass(VkCommandBuffer commandBuffer, unsigned int FrameID) const
{
	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.framebuffer = framebuffer;
	beginInfo.renderPass = render_pass;
	VkClearValue clearColor{};
	clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	static VkClearValue clearValues[] = { clearColor };
	beginInfo.clearValueCount = 1;
	beginInfo.pClearValues = clearValues;
	beginInfo.renderArea.extent.height = SmoothieCore::getScrHeight();
	beginInfo.renderArea.extent.width = SmoothieCore::getScrWidth();
	beginInfo.renderArea.offset = { 0, 0 };
	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Smoothie::DeferredRendering::Lighting_RenderPass::unbindPass(VkCommandBuffer commandBuffer, unsigned int FrameID) const
{
	vkCmdEndRenderPass(commandBuffer);
}

int Smoothie::DeferredRendering::Global_Illumination::create()
{
	if (renderPass == nullptr)
	{
		std::cout << "Render pass object is not valid!" << std::endl;
		return 1;
	}

	if (DrawClassSetLayout == nullptr)
	{
		std::cout << "Draw class descriptor set layout is not valid!" << std::endl;
		return 1;
	}

	const VkDescriptorSetLayout descriptorSets[] =
	{
		SmoothieCore::getCameraDescriptorSetLayout(0),
		DrawClassSetLayout,
		gBufferImagesDescriptorSetLayout,
		BRDF_DescriptorSetLayout,
		IrradiancePrefilter_DescriptorSetLayout
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = sizeof(descriptorSets)/sizeof(descriptorSets[0]);
	pipelineLayoutInfo.pSetLayouts = descriptorSets;
	if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != 0)
	{
		std::cout << "Failed to create pipeline layout!" << std::endl;
		return 1;
	}


	VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
	vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderPipelineCreateInfo.pName = "main";
	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderPipelineCreateInfo.module = vertexShader;

	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderPipelineCreateInfo.pName = "main";
	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderPipelineCreateInfo.module = globalIlluminationModule;

	const VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;
	Smoothie::DefaultPipelineState __state;
	__state.populate_pipeline(pipelineInfo);
	pipelineInfo.pDepthStencilState;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.layout = pipelineLayout;
	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), nullptr, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create global illumination pipeline!" << std::endl;
		return 1;
	}

	return 0;
}

void Smoothie::DeferredRendering::Global_Illumination::destroy()
{
	vkDestroyPipeline(SmoothieCore::getDevice(), pipeline, nullptr), pipeline = nullptr;
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), pipelineLayout, nullptr), pipelineLayout = nullptr;
}

void Smoothie::DeferredRendering::Global_Illumination::draw(VkCommandBuffer commandBuffer, VkDescriptorSet drawClassSet, unsigned int ImageID) const
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	SmoothieCore::setViewport(commandBuffer);
	SmoothieCore::setScissor(commandBuffer);

	const VkDescriptorSet _descriptors[5] =
	{
		SmoothieCore::getCameraDescriptorSet(),
		drawClassSet,
		gBufferImagesDescriptorSet,
		BRDF_DescriptorSet,
		IrradiancePrefilter_DescriptorSet
	};

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 5, _descriptors, 0, 0);
	vkCmdDraw(commandBuffer, 6, 1, 0, 0);
}

int Smoothie::DeferredRendering::Global_Illumination::resize_callback()
{
	return 0;
}

int Smoothie::DeferredRendering::BRDF::create()
{
	if (Texture2DModelSampler == nullptr)
	{
		std::cout << "Texture2DModelSampler is not a valid sampler object!" << std::endl;
		return 1;
	}


	//************************************* BRDF Image ****************************************************//
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
	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &createImage, &allocInfo, &BRDImage, &BRDFallocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create image!" << std::endl;
		return 1;
	}

	//************************************* BRDF Image View ****************************************************//
	VkImageViewCreateInfo createImageView{};
	createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createImageView.image = BRDImage;
	createImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createImageView.format = createImage.format;
	createImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createImageView.subresourceRange.levelCount = 1;
	createImageView.subresourceRange.layerCount = 1;
	if (vkCreateImageView(SmoothieCore::getDevice(), &createImageView, nullptr, &BRDFImageView) != VK_SUCCESS)
	{
		std::cout << "Failed to create image view!" << std::endl;
		return 1;
	}


	//************************************* BRDF Render pass ****************************************************//
	VkRenderPass renderPass = nullptr;
	VkAttachmentDescription attachmentDescription{};
	attachmentDescription.format = createImage.format;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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
	if (vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		std::cout << "Failed to create render pass!" << std::endl;
		return 1;
	}


	//************************************* BRDF Framebuffer ****************************************************//
	VkFramebuffer framebuffer = nullptr;
	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = renderPass;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &BRDFImageView;
	createInfo.width = createImage.extent.width;
	createInfo.height = createImage.extent.height;
	createInfo.layers = createImage.extent.depth;
	if (vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &framebuffer) != VK_SUCCESS)
	{
		std::cout << "Failed to create framebuffer!" << std::endl;
		return 1;
	}


	//************************************* BRDF Pipelines ****************************************************//
	VkPipelineLayout pipelineLayout = nullptr;
	VkPipeline pipeline = nullptr;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != 0)
	{
		std::cout << "Failed to create pipeline layout!" << std::endl;
		return 1;
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
	vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderPipelineCreateInfo.pName = "main";
	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderPipelineCreateInfo.module = vertexShader;
	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderPipelineCreateInfo.pName = "main";
	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderPipelineCreateInfo.module = fragmentShader;
	const VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;
	DefaultPipelineState _pipe;
	_pipe.populate_pipeline(pipelineInfo);
	const VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;
	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create graphics pipeline!" << std::endl;
		return 1;
	}


	//************************************* BRDF rendering ****************************************************//
	auto commandBuffer = beginSingleTimeCommands();
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.framebuffer = framebuffer;
	renderPassBeginInfo.renderPass = renderPass;
	VkClearValue clearColor{};
	clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearColor;
	renderPassBeginInfo.renderArea.extent.height = createImage.extent.height;
	renderPassBeginInfo.renderArea.extent.width = createImage.extent.width;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	vkCmdBindPipeline(commandBuffer.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBeginRenderPass(commandBuffer.buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(createImage.extent.height);
	viewport.height = static_cast<float>(createImage.extent.width);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer.buffer, 0, 1, &viewport);
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent.height = createImage.extent.height;
	scissor.extent.width = createImage.extent.height;
	vkCmdSetScissor(commandBuffer.buffer, 0, 1, &scissor);
	vkCmdDraw(commandBuffer.buffer, 6, 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer.buffer);
	endSingleTimeCommands(commandBuffer);


	//************************************* Cleanup for unused stuff ***************************************************//
	vkDestroyPipeline(SmoothieCore::getDevice(), pipeline, nullptr); pipeline = nullptr;
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), pipelineLayout, nullptr); pipelineLayout = nullptr;
	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr); framebuffer = nullptr;
	vkDestroyRenderPass(SmoothieCore::getDevice(), renderPass, nullptr); renderPass = nullptr;


	//************************************* Descriptor layout ***************************************************//
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

	//************************************* Descriptor pool and set ***************************************************//
	VkDescriptorPoolSize texturePoolSize{};
	texturePoolSize.type = binding.descriptorType;
	texturePoolSize.descriptorCount = 1;
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	const VkDescriptorPoolSize pool_sizes[] = { texturePoolSize };
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = pool_sizes;
	poolInfo.maxSets = 1;
	if (vkCreateDescriptorPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor pool!" << std::endl;
		return 1;
	}

	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
	if (vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &descriptorSet) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor set!" << std::endl;
		return 1;
	}

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = BRDFImageView;
	imageInfo.sampler = Texture2DModelSampler;

	VkWriteDescriptorSet _WriteIrradinaceSet{};
	_WriteIrradinaceSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	_WriteIrradinaceSet.dstSet = descriptorSet;
	_WriteIrradinaceSet.dstArrayElement = 0;
	_WriteIrradinaceSet.descriptorType = binding.descriptorType;
	_WriteIrradinaceSet.descriptorCount = 1;
	_WriteIrradinaceSet.pBufferInfo = nullptr;
	_WriteIrradinaceSet.pImageInfo = &imageInfo;
	_WriteIrradinaceSet.dstBinding = 0;
	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_WriteIrradinaceSet, 0, nullptr);


	return 0;
}

void Smoothie::DeferredRendering::BRDF::destroy()
{
	vkDestroyDescriptorPool(SmoothieCore::getDevice(), descriptorPool, nullptr), descriptorPool = nullptr, descriptorSet = nullptr;
	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), descriptorSetLayout, nullptr), descriptorSetLayout = nullptr;
	vkDestroyImageView(SmoothieCore::getDevice(), BRDFImageView, nullptr), BRDFImageView = nullptr;
	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), BRDImage, BRDFallocation);
	BRDImage = nullptr, BRDFallocation = nullptr;
}

int Smoothie::DeferredRendering::IndirectLightingMaps::create()
{
	if (Texture2DModelSampler == nullptr)
	{
		std::cout << "Texture2DModelSampler is not a valid sampler object!" << std::endl;
		return 1;
	}

	//**************************************** Irradiance image ****************************************//
	VkImageCreateInfo _IrrMapCreateInfo{};
	_IrrMapCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	_IrrMapCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	_IrrMapCreateInfo.extent.width = 32;
	_IrrMapCreateInfo.extent.height = 32;
	_IrrMapCreateInfo.extent.depth = 1;
	_IrrMapCreateInfo.mipLevels = 1;
	_IrrMapCreateInfo.arrayLayers = 6;
	_IrrMapCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	_IrrMapCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_IrrMapCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	_IrrMapCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	_IrrMapCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	_IrrMapCreateInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	_IrrMapCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.priority = 1.0f;
	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_IrrMapCreateInfo, &allocInfo, &IrradianceMapImage, &IrradianceMapAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create irradiance map image!" << std::endl;
		return 1;
	}

	VkImageViewCreateInfo _IrrMapViewCreateInfo{};
	_IrrMapViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	_IrrMapViewCreateInfo.image = IrradianceMapImage;
	_IrrMapViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	_IrrMapViewCreateInfo.format = _IrrMapCreateInfo.format;
	_IrrMapViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	_IrrMapViewCreateInfo.subresourceRange.baseMipLevel = 0;
	_IrrMapViewCreateInfo.subresourceRange.levelCount = 1;
	_IrrMapViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	_IrrMapViewCreateInfo.subresourceRange.layerCount = 6;
	if (vkCreateImageView(SmoothieCore::getDevice(), &_IrrMapViewCreateInfo, nullptr, &IrradianceMapImageView) != VK_SUCCESS)
	{
		std::cout << "Failed to create image map image view!" << std::endl;
		return 1;
	}

	auto commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = IrradianceMapImage;
	barrier.oldLayout = _IrrMapCreateInfo.initialLayout;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.subresourceRange = _IrrMapViewCreateInfo.subresourceRange;
	vkCmdPipelineBarrier(commandBuffer.buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	VkClearColorValue _clearColorValue{};
	_clearColorValue.float32[0] = 0.69f;
	_clearColorValue.float32[1] = 0.69f;
	_clearColorValue.float32[2] = 0.69f;
	_clearColorValue.float32[3] = 1.0f;
	vkCmdClearColorImage(commandBuffer.buffer, IrradianceMapImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &_clearColorValue, 1, &_IrrMapViewCreateInfo.subresourceRange);
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = 0;
	vkCmdPipelineBarrier(commandBuffer.buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	
	endSingleTimeCommands(commandBuffer);

	//**************************************** Prefilter image ****************************************//
	VkImageCreateInfo _PrefImageCreateImage{};
	_PrefImageCreateImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	_PrefImageCreateImage.imageType = VK_IMAGE_TYPE_2D;
	_PrefImageCreateImage.extent.width = 128;
	_PrefImageCreateImage.extent.height = 128;
	_PrefImageCreateImage.extent.depth = 1;
	_PrefImageCreateImage.mipLevels = 5;
	_PrefImageCreateImage.arrayLayers = 6;
	_PrefImageCreateImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	_PrefImageCreateImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_PrefImageCreateImage.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	_PrefImageCreateImage.samples = VK_SAMPLE_COUNT_1_BIT;
	_PrefImageCreateImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	_PrefImageCreateImage.format = _IrrMapCreateInfo.format;
	_PrefImageCreateImage.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_PrefImageCreateImage, &allocInfo, &PrefilterMapImage, &PrefilterMapAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create prefilter image!" << std::endl;
		return 1;
	}

	VkImageViewCreateInfo _PrefImageCreateImageView{};
	_PrefImageCreateImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	_PrefImageCreateImageView.image = PrefilterMapImage;
	_PrefImageCreateImageView.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	_PrefImageCreateImageView.format = _PrefImageCreateImage.format;
	_PrefImageCreateImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	_PrefImageCreateImageView.subresourceRange.baseMipLevel = 0;
	_PrefImageCreateImageView.subresourceRange.levelCount = 5;
	_PrefImageCreateImageView.subresourceRange.baseArrayLayer = 0;
	_PrefImageCreateImageView.subresourceRange.layerCount = 6;
	if (vkCreateImageView(SmoothieCore::getDevice(), &_PrefImageCreateImageView, nullptr, &PrefilterMapImageView) != VK_SUCCESS)
	{
		std::cout << "Failed to create prefilter map image view!" << std::endl;
		return 1;
	}

	commandBuffer = beginSingleTimeCommands();

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = PrefilterMapImage;
	barrier.oldLayout = _PrefImageCreateImage.initialLayout;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.subresourceRange = _PrefImageCreateImageView.subresourceRange;
	vkCmdPipelineBarrier(commandBuffer.buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	_clearColorValue.float32[0] = 0.69f;
	_clearColorValue.float32[1] = 0.69f;
	_clearColorValue.float32[2] = 0.69f;
	_clearColorValue.float32[3] = 1.0f;
	vkCmdClearColorImage(commandBuffer.buffer, PrefilterMapImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &_clearColorValue, 1, &_PrefImageCreateImageView.subresourceRange);
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = 0;
	vkCmdPipelineBarrier(commandBuffer.buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	endSingleTimeCommands(commandBuffer);


	//**************************************** Descriptors ****************************************//
	VkDescriptorSetLayoutBinding _IrradianceMapBidings{};
	_IrradianceMapBidings.binding = 0;
	_IrradianceMapBidings.descriptorCount = 1;
	_IrradianceMapBidings.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	_IrradianceMapBidings.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	VkDescriptorSetLayoutBinding _PrefilterMapBidings = _IrradianceMapBidings;
	_PrefilterMapBidings.binding = 1;

	VkDescriptorSetLayoutCreateInfo _descriptorSetLayoutCreateInfo{};
	_descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	_descriptorSetLayoutCreateInfo.bindingCount = 2;
	VkDescriptorSetLayoutBinding _bindings[2] = { _IrradianceMapBidings , _PrefilterMapBidings };
	_descriptorSetLayoutCreateInfo.pBindings = _bindings;
	if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &_descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor set layout!" << std::endl;
		return 1;
	}

	VkDescriptorPoolCreateInfo _poolCreateInfo{};
	_poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	_poolCreateInfo.maxSets = 1;
	VkDescriptorPoolSize _poolSize = {};
	_poolSize.descriptorCount = 2;
	_poolSize.type = _IrradianceMapBidings.descriptorType;
	_poolCreateInfo.poolSizeCount = 1;
	_poolCreateInfo.pPoolSizes = &_poolSize;
	if (vkCreateDescriptorPool(SmoothieCore::getDevice(), &_poolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor pool!" << std::endl;
		return 1;
	}

	VkDescriptorSetAllocateInfo _allocInfo = {};
	_allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	_allocInfo.descriptorPool = descriptorPool;
	_allocInfo.descriptorSetCount = 1;
	_allocInfo.pSetLayouts = &descriptorSetLayout;
	vkAllocateDescriptorSets(SmoothieCore::getDevice(), &_allocInfo, &descriptorSet);


	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = IrradianceMapImageView;
	imageInfo.sampler = Texture2DModelSampler;

	VkWriteDescriptorSet _WriteIrradinaceSet{};
	_WriteIrradinaceSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	_WriteIrradinaceSet.dstSet = descriptorSet;
	_WriteIrradinaceSet.dstArrayElement = 0;
	_WriteIrradinaceSet.descriptorType = _IrradianceMapBidings.descriptorType;
	_WriteIrradinaceSet.descriptorCount = 1;
	_WriteIrradinaceSet.pBufferInfo = nullptr;
	_WriteIrradinaceSet.pImageInfo = &imageInfo;
	_WriteIrradinaceSet.dstBinding = 0;
	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_WriteIrradinaceSet, 0, nullptr);

	imageInfo.imageView = PrefilterMapImageView;
	_WriteIrradinaceSet.dstBinding = 1;
	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_WriteIrradinaceSet, 0, nullptr);

	return 0;
}

void generateViewMatrices(std::array<SmoothieMath::Matrix4x4, 6>& matrices);

struct _PrefilterHelperStruct
{
	VkImage image = nullptr;
	VkImageView imageView = nullptr;
	VmaAllocation allocation = nullptr;
};

int Smoothie::DeferredRendering::IndirectLightingMaps::create_maps_from_skybox_cubemap(const DeferredRendering::SkyboxCubemapTexture& skyboxCubemap)
{
	if ((IrradianceMapImage == nullptr) || (PrefilterMapImage == nullptr) || (descriptorSet == nullptr))
	{
		if (create() != 0)
		{
			std::cout << "Failed to create base Irradiange map or prefilter textures!" << std::endl;
			return 1;
		}
	}

	//************************************ Base objects needed for calculations ************************************//
	VkRenderPass renderPass = nullptr;
	VkAttachmentDescription attachmentDescription{};
	attachmentDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
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
	if (vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		std::cout << "Failed to create render pass!" << std::endl;
		return 1;
	}

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.priority = 1.0f;

	std::array<SmoothieMath::Matrix4x4, 6> viewMatrices;
	generateViewMatrices(viewMatrices);

	//***************************** Irradiance map render target and framebuffer *******************************//
	VkImage _IrrMapTargetImage = nullptr;
	VkImageView _IrrMapTargetImageView = nullptr;
	VmaAllocation _IrrMapTargetAllocation = nullptr;
	VkImageCreateInfo _IrrMapTargetImageCreateInfo{};
	_IrrMapTargetImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	_IrrMapTargetImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	_IrrMapTargetImageCreateInfo.extent = { 32 , 32 , 1 };
	_IrrMapTargetImageCreateInfo.mipLevels = 1;
	_IrrMapTargetImageCreateInfo.arrayLayers = 1;
	_IrrMapTargetImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	_IrrMapTargetImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_IrrMapTargetImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	_IrrMapTargetImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	_IrrMapTargetImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	_IrrMapTargetImageCreateInfo.format = attachmentDescription.format;
	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_IrrMapTargetImageCreateInfo, &allocInfo, &_IrrMapTargetImage, &_IrrMapTargetAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create irradiance map render target image!" << std::endl;
		return 1;
	}

	VkImageViewCreateInfo _IrrMapTargetImageViewCreateInfo{};
	_IrrMapTargetImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	_IrrMapTargetImageViewCreateInfo.image = _IrrMapTargetImage;
	_IrrMapTargetImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	_IrrMapTargetImageViewCreateInfo.format = attachmentDescription.format;
	_IrrMapTargetImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	_IrrMapTargetImageViewCreateInfo.subresourceRange.levelCount = 1;
	_IrrMapTargetImageViewCreateInfo.subresourceRange.layerCount = 1;
	if (vkCreateImageView(SmoothieCore::getDevice(), &_IrrMapTargetImageViewCreateInfo, nullptr, &_IrrMapTargetImageView) != VK_SUCCESS)
	{
		std::cout << "Failed to create irradiance map render target image view!" << std::endl;
		return 1;
	}

	VkFramebuffer _IrradianceMapFramebuffer = nullptr;
	VkFramebufferCreateInfo _IrrFramebufferCreateInfo{};
	_IrrFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	_IrrFramebufferCreateInfo.renderPass = renderPass;
	_IrrFramebufferCreateInfo.attachmentCount = 1;
	_IrrFramebufferCreateInfo.pAttachments = &_IrrMapTargetImageView;
	_IrrFramebufferCreateInfo.width = _IrrMapTargetImageCreateInfo.extent.width;
	_IrrFramebufferCreateInfo.height = _IrrMapTargetImageCreateInfo.extent.height;
	_IrrFramebufferCreateInfo.layers = 1;
	if (vkCreateFramebuffer(SmoothieCore::getDevice(), &_IrrFramebufferCreateInfo, nullptr, &_IrradianceMapFramebuffer) != VK_SUCCESS)
	{
		std::cout << "Failed to create irradiance map framebuffer!" << std::endl;
		return 1;
	}


	//*************************************** Irradiance map pipeline ***************************************//
	VkPipelineLayout pipelineLayout = nullptr;
	VkPipeline pipeline = nullptr;
	VkPushConstantRange range = {};
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	range.offset = 0;
	range.size = sizeof(SmoothieMath::Matrix4x4);
	VkPipelineLayoutCreateInfo _IrrPipelineLayoutCreateInfo{};
	_IrrPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	_IrrPipelineLayoutCreateInfo.setLayoutCount = 1;
	auto __layout = skyboxCubemap.getDescriptorSetLayout();
	_IrrPipelineLayoutCreateInfo.pSetLayouts = &__layout;
	_IrrPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	_IrrPipelineLayoutCreateInfo.pPushConstantRanges = &range;
	if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &_IrrPipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create pipeline layout for irradiance calculation!" << std::endl;
		return 1;
	}


	VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
	vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderPipelineCreateInfo.pName = "main";
	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderPipelineCreateInfo.module = pbsVertexModule;

	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderPipelineCreateInfo.pName = "main";
	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderPipelineCreateInfo.module = irradianceMapModule;

	VkGraphicsPipelineCreateInfo _IrrMapPipelineCreateInfo{};
	_IrrMapPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
	_IrrMapPipelineCreateInfo.stageCount = 2;
	_IrrMapPipelineCreateInfo.pStages = stages;
	Smoothie::DefaultPipelineState __default_state;
	__default_state.populate_pipeline(_IrrMapPipelineCreateInfo);
	_IrrMapPipelineCreateInfo.layout = pipelineLayout;
	_IrrMapPipelineCreateInfo.renderPass = renderPass;
	_IrrMapPipelineCreateInfo.subpass = 0;
	_IrrMapPipelineCreateInfo.basePipelineHandle = nullptr;
	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &_IrrMapPipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create graphics pipelines!" << std::endl;
		return 1;
	}


	//*************************************** Irradiance map Calculation ***************************************//
	auto commandBuffer = beginSingleTimeCommands();
	VkImageMemoryBarrier _ToTransferOnlyBarrier{};
	_ToTransferOnlyBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	_ToTransferOnlyBarrier.srcAccessMask = 0;
	_ToTransferOnlyBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	_ToTransferOnlyBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_ToTransferOnlyBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	_ToTransferOnlyBarrier.image = IrradianceMapImage;
	_ToTransferOnlyBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	_ToTransferOnlyBarrier.subresourceRange.baseMipLevel = 0;
	_ToTransferOnlyBarrier.subresourceRange.levelCount = 1;
	_ToTransferOnlyBarrier.subresourceRange.baseArrayLayer = 0;
	_ToTransferOnlyBarrier.subresourceRange.layerCount = 6;
	vkCmdPipelineBarrier(commandBuffer.buffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &_ToTransferOnlyBarrier);


	vkCmdBindPipeline(commandBuffer.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(32);
	viewport.height = static_cast<float>(32);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer.buffer, 0, 1, &viewport);
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { 32, 32 };
	vkCmdSetScissor(commandBuffer.buffer, 0, 1, &scissor);
	const auto __set = skyboxCubemap.getDescriptorSet();
	vkCmdBindDescriptorSets(commandBuffer.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &__set, 0, nullptr);
	for (int i = 0; i < 6; i++)
	{

		VkRenderPassBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		beginInfo.framebuffer = _IrradianceMapFramebuffer;
		beginInfo.renderPass = renderPass;
		VkClearValue clearColor{};
		clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		beginInfo.clearValueCount = 1;
		beginInfo.pClearValues = &clearColor;
		beginInfo.renderArea.extent = { 32, 32 };
		beginInfo.renderArea.offset = { 0, 0 };
		vkCmdBeginRenderPass(commandBuffer.buffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdPushConstants(commandBuffer.buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SmoothieMath::Matrix4x4), &viewMatrices[i]);
		vkCmdDraw(commandBuffer.buffer, 36, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffer.buffer);

		//Wait for all graphics to finish its work
		VkMemoryBarrier _memory_barrier = {};
		_memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		vkCmdPipelineBarrier(commandBuffer.buffer,
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			1, &_memory_barrier,
			0, nullptr,
			0, nullptr);

		VkImageCopy copyRegions{};
		copyRegions.extent = { 32, 32, 1 };
		copyRegions.srcOffset = copyRegions.dstOffset = { 0, 0, 0 };
		copyRegions.srcSubresource.baseArrayLayer = 0;
		copyRegions.srcSubresource.layerCount = 1;
		copyRegions.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegions.dstSubresource.baseArrayLayer = i;
		copyRegions.dstSubresource.layerCount = 1;
		copyRegions.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		vkCmdCopyImage(commandBuffer.buffer, _IrrMapTargetImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, IrradianceMapImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegions);
	

		//Wait for all copying to finish its work before drawing again
		_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkCmdPipelineBarrier(commandBuffer.buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			0,
			1, &_memory_barrier,
			0, nullptr,
			0, nullptr);
	
	}

	
	VkImageMemoryBarrier __ToShaderReadOnlyBarrier{};
	__ToShaderReadOnlyBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	__ToShaderReadOnlyBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	__ToShaderReadOnlyBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	__ToShaderReadOnlyBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	__ToShaderReadOnlyBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	__ToShaderReadOnlyBarrier.image = IrradianceMapImage;
	__ToShaderReadOnlyBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	__ToShaderReadOnlyBarrier.subresourceRange.baseMipLevel = 0;
	__ToShaderReadOnlyBarrier.subresourceRange.levelCount = 1;
	__ToShaderReadOnlyBarrier.subresourceRange.baseArrayLayer = 0;
	__ToShaderReadOnlyBarrier.subresourceRange.layerCount = 6;
	vkCmdPipelineBarrier(commandBuffer.buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &__ToShaderReadOnlyBarrier);
	endSingleTimeCommands(commandBuffer);

	//clean resources that are no longer needed
	vkDestroyPipeline(SmoothieCore::getDevice(), pipeline, nullptr), pipeline = nullptr;
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), pipelineLayout, nullptr), pipelineLayout = nullptr;
	vkDestroyFramebuffer(SmoothieCore::getDevice(), _IrradianceMapFramebuffer, nullptr), _IrradianceMapFramebuffer = nullptr;
	vkDestroyImageView(SmoothieCore::getDevice(), _IrrMapTargetImageView, nullptr), _IrrMapTargetImageView = nullptr;
	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), _IrrMapTargetImage, _IrrMapTargetAllocation), _IrrMapTargetAllocation = nullptr, _IrrMapTargetImage = nullptr;
	
	
	//*************************************** Prefilter map targets and framebuffer ***************************************//
	std::array<_PrefilterHelperStruct, 5> tempImageMipChain;
	for (size_t mip = 0; mip < tempImageMipChain.size(); mip++)
	{
		const unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		VkImageCreateInfo __MipImageCreateInfo = _IrrMapTargetImageCreateInfo;
		__MipImageCreateInfo.extent = { mipWidth, mipWidth, 1 };
		auto& mipData = tempImageMipChain[mip];
		if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &__MipImageCreateInfo, &allocInfo, &mipData.image, &mipData.allocation, nullptr) != VK_SUCCESS)
		{
			std::cout << "Failed to create mip chain image: " << mip << "!" << std::endl;
			return 1;
		}

		VkImageViewCreateInfo __MipImageViewCreateInfo = _IrrMapTargetImageViewCreateInfo;
		__MipImageViewCreateInfo.image = mipData.image;

		if (vkCreateImageView(SmoothieCore::getDevice(), &__MipImageViewCreateInfo, nullptr, &mipData.imageView) != VK_SUCCESS)
		{
			std::cout << "Failed to create mip chain image view: " << mip << "!" << std::endl;
			return 1;
		}

	}
	
	std::array<VkFramebuffer, 5> mipChainFramebuffers = { nullptr };
	for (size_t mip = 0; mip < mipChainFramebuffers.size(); mip++)
	{
		const unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		VkFramebufferCreateInfo __MipChainFramebufferCreateInfo = _IrrFramebufferCreateInfo;
		__MipChainFramebufferCreateInfo.width = __MipChainFramebufferCreateInfo.height = mipWidth;
		__MipChainFramebufferCreateInfo.pAttachments = &tempImageMipChain[mip].imageView;
		if (vkCreateFramebuffer(SmoothieCore::getDevice(), &__MipChainFramebufferCreateInfo, nullptr, &mipChainFramebuffers[mip]) != VK_SUCCESS)
		{
			std::cout << "Failed to create swapchain framebuffer: " << mip << "!" << std::endl;
			return 1;
		}
	}

	//*************************************** Prefilter map pipelines ***************************************//
	range.size = sizeof(SmoothieMath::Matrix4x4) + sizeof(float);
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &_IrrPipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create pipeline layout for prefilter calculation!" << std::endl;
		return 1;
	}

	fragmentShaderPipelineCreateInfo.module = prefilterMapModule;
	_IrrMapPipelineCreateInfo.layout = pipelineLayout;

	VkPipelineShaderStageCreateInfo __stages[2] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
	_IrrMapPipelineCreateInfo.pStages = __stages;
	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &_IrrMapPipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create pipeline for prefilter calculation!" << std::endl;
		return 1;
	}

	//*************************************** Prefilter map calculation ***************************************//

	commandBuffer = beginSingleTimeCommands();
	_ToTransferOnlyBarrier.image = PrefilterMapImage;
	_ToTransferOnlyBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	_ToTransferOnlyBarrier.subresourceRange.levelCount = 5;
	vkCmdPipelineBarrier(commandBuffer.buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &_ToTransferOnlyBarrier);
	vkCmdBindPipeline(commandBuffer.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindDescriptorSets(commandBuffer.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &__set, 0, nullptr);
	for (int layer = 0; layer < 6; layer++)
	{
		for (unsigned int mip = 0; mip < tempImageMipChain.size(); mip++)
		{
			unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));

			VkRenderPassBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			beginInfo.framebuffer = mipChainFramebuffers[mip];
			beginInfo.renderPass = renderPass;
			VkClearValue clearColor{};
			clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };
			beginInfo.clearValueCount = 1;
			beginInfo.pClearValues = &clearColor;
			beginInfo.renderArea.extent = { mipWidth, mipWidth };
			beginInfo.renderArea.offset = { 0, 0 };
			vkCmdBeginRenderPass(commandBuffer.buffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(mipWidth);
			viewport.height = static_cast<float>(mipWidth);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer.buffer, 0, 1, &viewport);
			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = { mipWidth, mipWidth };
			vkCmdSetScissor(commandBuffer.buffer, 0, 1, &scissor);

			struct PrefilterPushConstantHelper
			{
				SmoothieMath::Matrix4x4 view;
				float roughness = 0;
			};

			PrefilterPushConstantHelper __helper;
			__helper.roughness = (float)mip / (float)(mipChainFramebuffers.size() - 1);
			__helper.view = viewMatrices[layer];
			vkCmdPushConstants(commandBuffer.buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PrefilterPushConstantHelper), &__helper);

			vkCmdDraw(commandBuffer.buffer, 36, 1, 0, 0);
			vkCmdEndRenderPass(commandBuffer.buffer);

			//Wait for all graphics to finish its work
			VkMemoryBarrier _memory_barrier = {};
			_memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			vkCmdPipelineBarrier(commandBuffer.buffer,
				VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				1, &_memory_barrier,
				0, nullptr,
				0, nullptr);

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
			vkCmdCopyImage(commandBuffer.buffer, tempImageMipChain[mip].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, PrefilterMapImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegions);
		

			//Wait for all copying to finish its work before drawing again
			_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			vkCmdPipelineBarrier(commandBuffer.buffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
				0,
				1, &_memory_barrier,
				0, nullptr,
				0, nullptr);

		}

	}

	__ToShaderReadOnlyBarrier.image = PrefilterMapImage;
	__ToShaderReadOnlyBarrier.subresourceRange.levelCount = 5;
	vkCmdPipelineBarrier(commandBuffer.buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &__ToShaderReadOnlyBarrier);
	endSingleTimeCommands(commandBuffer);


	//Cleanup 
	vkDestroyPipeline(SmoothieCore::getDevice(), pipeline, nullptr), pipeline = nullptr;
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), pipelineLayout, nullptr);
	for (size_t i = 0; i < tempImageMipChain.size(); i++)
	{
		vkDestroyFramebuffer(SmoothieCore::getDevice(), mipChainFramebuffers[i], nullptr), mipChainFramebuffers[i] = nullptr;
		auto& mip = tempImageMipChain[i];
		vkDestroyImageView(SmoothieCore::getDevice(), mip.imageView, nullptr), mip.imageView = nullptr;
		vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), mip.image, mip.allocation);
		mip.image = nullptr, mip.allocation = nullptr;
	}
	vkDestroyRenderPass(SmoothieCore::getDevice(), renderPass, nullptr);

	return 0;
}

void Smoothie::DeferredRendering::IndirectLightingMaps::destroy()
{
	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), IrradianceMapImage, IrradianceMapAllocation), IrradianceMapImage = nullptr, IrradianceMapAllocation = nullptr;
	vkDestroyImageView(SmoothieCore::getDevice(), IrradianceMapImageView, nullptr);
	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), PrefilterMapImage, PrefilterMapAllocation), PrefilterMapImage = nullptr, PrefilterMapAllocation = nullptr;
	vkDestroyImageView(SmoothieCore::getDevice(), PrefilterMapImageView, nullptr), PrefilterMapImageView = nullptr;
	vkDestroyDescriptorPool(SmoothieCore::getDevice(), descriptorPool, nullptr), descriptorPool = nullptr, descriptorSet = nullptr;
	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), descriptorSetLayout, nullptr), descriptorSetLayout = nullptr;
}
