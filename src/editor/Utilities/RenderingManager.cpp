// #include "RenderingManager.h"
//
// //This is only here because I want to completely disable tab bar in my main window.
// //By the looks of its its very early API so I pray to God that this things won't change in the future.
// #include "imgui_internal.h"
//
// using namespace SmoothieEditor;
//
// int SmoothieEditor::Swizzler::create()
// {
// 	VkDescriptorPoolSize __srcImagePoolSize = {};
// 	__srcImagePoolSize.descriptorCount = 1;
// 	__srcImagePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//
// 	VkDescriptorPoolSize __dstImagePoolSize = {};
// 	__dstImagePoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
// 	__dstImagePoolSize.descriptorCount = 1;
//
// 	const VkDescriptorPoolSize __poolSizes[] = { __srcImagePoolSize, __dstImagePoolSize };
// 	VkDescriptorPoolCreateInfo __poolCreateInfo = {};
// 	__poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
// 	__poolCreateInfo.maxSets = 1;
// 	__poolCreateInfo.poolSizeCount = 2;
// 	__poolCreateInfo.pPoolSizes = __poolSizes;
// 	if(vkCreateDescriptorPool(SmoothieCore::getDevice(), &__poolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create descriptor pool!" << std::endl;
// 		return 1;
// 	}
//
// 	VkDescriptorSetLayoutBinding __srcImageSetBinding = {};
// 	__srcImageSetBinding.binding = 0;
// 	__srcImageSetBinding.descriptorCount = 1;
// 	__srcImageSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// 	__srcImageSetBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
//
// 	VkDescriptorSetLayoutBinding __dstImageSetBinding = {};
// 	__dstImageSetBinding.binding = 1;
// 	__dstImageSetBinding.descriptorCount = 1;
// 	__dstImageSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
// 	__dstImageSetBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
//
//
// 	const VkDescriptorSetLayoutBinding __descriptorSetBindings[] = { __srcImageSetBinding , __dstImageSetBinding };
// 	VkDescriptorSetLayoutCreateInfo __setLayoutCreateInfo = {};
// 	__setLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
// 	__setLayoutCreateInfo.bindingCount = 2;
// 	__setLayoutCreateInfo.pBindings = __descriptorSetBindings;
// 	if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &__setLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create descriptor set layout!" << std::endl;
// 		return 1;
// 	}
//
// 	VkDescriptorSetAllocateInfo __allocInfo = {};
// 	__allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
// 	__allocInfo.descriptorPool = descriptorPool;
// 	__allocInfo.descriptorSetCount = 1;
// 	__allocInfo.pSetLayouts = &descriptorSetLayout;
// 	if (vkAllocateDescriptorSets(SmoothieCore::getDevice(), &__allocInfo, &descriptorSet) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to allocate descriptor set!" << std::endl;
// 		return 1;
// 	}
//
// 	VkPushConstantRange __push_constant = {};
// 	__push_constant.offset = 0;
// 	__push_constant.size = sizeof(glm::mat4);
// 	__push_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
//
// 	VkPipelineLayoutCreateInfo __pipelineLayoutCreateInfo = {};
// 	__pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
// 	__pipelineLayoutCreateInfo.setLayoutCount = 1;
// 	__pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
// 	__pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
// 	__pipelineLayoutCreateInfo.pPushConstantRanges = &__push_constant;
// 	if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &__pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create pipeline layout!" << std::endl;
// 		return 1;
// 	}
//
// 	VkPipelineShaderStageCreateInfo __shaderCreateInfo = {};
// 	__shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
// 	__shaderCreateInfo.pName = "main";
// 	__shaderCreateInfo.module = shader;
// 	__shaderCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
//
// 	VkComputePipelineCreateInfo __pipelineCreateInfo = {};
// 	__pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
// 	__pipelineCreateInfo.layout = pipelineLayout;
// 	__pipelineCreateInfo.stage = __shaderCreateInfo;
// 	if (vkCreateComputePipelines(SmoothieCore::getDevice(), nullptr, 1, &__pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create pipeline!" << std::endl;
// 		return 1;
// 	}
//
// 	return 0;
// }
//
// void SmoothieEditor::Swizzler::draw(VkCommandBuffer commandBuffer) const
// {
// 	VkImageMemoryBarrier __barrier = {};
// 	__barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
// 	__barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 	__barrier.subresourceRange.baseArrayLayer = 0;
// 	__barrier.subresourceRange.layerCount = 1;
// 	__barrier.subresourceRange.baseMipLevel = 0;
// 	__barrier.subresourceRange.levelCount = 1;
//
// 	__barrier.image = dstImage;
//
// 	__barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
// 	__barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
// 	__barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 	__barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
//
// 	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
// 		0, nullptr,
// 		0, nullptr,
// 		1, &__barrier);
//
// 	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
// 	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(glm::mat4), &matrix);
// 	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
// 	vkCmdDispatch(commandBuffer, SmoothieCore::getScrWidth(), SmoothieCore::getScrHeight(), 1);
//
// 	__barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
// 	__barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
// 	__barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
// 	__barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
// 		0, nullptr,
// 		0, nullptr,
// 		1, &__barrier);
// }
//
// void SmoothieEditor::Swizzler::update_source_image(VkImage image, VkImageView imageView)
// {
// 	vkQueueWaitIdle(SmoothieCore::getGraphicsQueue());
//
// 	srcImage = image;
// 	srcImageView = imageView;
//
// 	VkDescriptorImageInfo imgInfo{};
// 	imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 	imgInfo.imageView = imageView;
// 	imgInfo.sampler = sampler;
//
// 	VkWriteDescriptorSet descriptorWrite{};
// 	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// 	descriptorWrite.dstSet = descriptorSet;
// 	descriptorWrite.dstArrayElement = 0;
// 	descriptorWrite.dstBinding = 0;
// 	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// 	descriptorWrite.descriptorCount = 1;
// 	descriptorWrite.pBufferInfo = nullptr;
// 	descriptorWrite.pImageInfo = &imgInfo;
// 	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
// }
//
// void SmoothieEditor::Swizzler::update_destination_image(VkImage image, VkImageView imageView)
// {
// 	vkQueueWaitIdle(SmoothieCore::getGraphicsQueue());
//
// 	dstImage = image;
// 	dstImageView = imageView;
//
// 	VkDescriptorImageInfo imgInfo{};
// 	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
// 	imgInfo.imageView = imageView;
// 	imgInfo.sampler = sampler;
//
// 	VkWriteDescriptorSet descriptorWrite{};
// 	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// 	descriptorWrite.dstSet = descriptorSet;
// 	descriptorWrite.dstArrayElement = 0;
// 	descriptorWrite.dstBinding = 1;
// 	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
// 	descriptorWrite.descriptorCount = 1;
// 	descriptorWrite.pBufferInfo = nullptr;
// 	descriptorWrite.pImageInfo = &imgInfo;
// 	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
// }
//
// void SmoothieEditor::Swizzler::destroy()
// {
// 	vkDestroyPipeline(SmoothieCore::getDevice(), pipeline, nullptr), pipeline = nullptr;
// 	vkDestroyPipelineLayout(SmoothieCore::getDevice(), pipelineLayout, nullptr), pipelineLayout = nullptr;
// 	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), descriptorSetLayout, nullptr), descriptorSetLayout = nullptr;
// 	vkDestroyDescriptorPool(SmoothieCore::getDevice(), descriptorPool, nullptr);
// 	descriptorPool = nullptr, descriptorSet = nullptr;
// }
//
// int SmoothieEditor::RenderingManager::create_imgui_data()
// {
// 	for (const auto& _shader: __editor_shader_files)
// 	{
// 		// if (Smoothie::Add_system_shader(_shader, editor_shaders) != 0)
// 		// {
// 		// 	std::cout << "Failed to create shader modules!" << std::endl;
// 		// 	return 1;
// 		// }
// 	}
//
// 	VkAttachmentDescription description{};
// 	description.format = VK_FORMAT_R8G8B8A8_UNORM;
// 	description.samples = VK_SAMPLE_COUNT_1_BIT;
// 	description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
// 	description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
// 	description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
// 	description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
// 	description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 	description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 	VkAttachmentReference colorAttachmentRef{};
// 	colorAttachmentRef.attachment = 0;
// 	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
// 	VkSubpassDescription subpass = {};
// 	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
// 	subpass.colorAttachmentCount = 1;
// 	subpass.pColorAttachments = &colorAttachmentRef;
// 	subpass.pDepthStencilAttachment = nullptr;
// 	VkRenderPassCreateInfo renderPassInfo = {};
// 	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
// 	renderPassInfo.attachmentCount = 1;
// 	renderPassInfo.pAttachments = &description;
// 	renderPassInfo.subpassCount = 1;
// 	renderPassInfo.pSubpasses = &subpass;
// 	renderPassInfo.dependencyCount = 0;
// 	renderPassInfo.pDependencies = nullptr;
// 	if (vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &resultRenderPass) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create result render pass!" << std::endl;
// 		return 1;
// 	}
//
// 	VkImageCreateInfo _resultImageCreateInfo{};
// 	_resultImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
// 	_resultImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
// 	_resultImageCreateInfo.extent.width = SmoothieCore::getScrWidth();
// 	_resultImageCreateInfo.extent.height = SmoothieCore::getScrHeight();
// 	_resultImageCreateInfo.extent.depth = 1;
// 	_resultImageCreateInfo.mipLevels = 1;
// 	_resultImageCreateInfo.arrayLayers = 1;
// 	_resultImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
// 	_resultImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 	_resultImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
// 	_resultImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
// 	_resultImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
// 	_resultImageCreateInfo.format = description.format;
// 	VmaAllocationCreateInfo vmaImageAllocationInfo{};
// 	vmaImageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
// 	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_resultImageCreateInfo, &vmaImageAllocationInfo, &resultImage, &resultImageAllocation, nullptr) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create result image!" << std::endl;
// 		return 1;
// 	}
//
// 	VkImageViewCreateInfo _resultImageViewCreateInfo{};
// 	_resultImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
// 	_resultImageViewCreateInfo.image = resultImage;
// 	_resultImageViewCreateInfo.format = _resultImageCreateInfo.format;
// 	_resultImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
// 	_resultImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 	_resultImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
// 	_resultImageViewCreateInfo.subresourceRange.levelCount = 1;
// 	_resultImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
// 	_resultImageViewCreateInfo.subresourceRange.layerCount = 1;
// 	if (vkCreateImageView(SmoothieCore::getDevice(), &_resultImageViewCreateInfo, nullptr, &resultImageView) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create Image view for result!" << std::endl;
// 		return 1;
// 	}
//
// 	VkFramebufferCreateInfo _ResultFramebufferInfo{};
// 	_ResultFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
// 	_ResultFramebufferInfo.renderPass = resultRenderPass;
// 	_ResultFramebufferInfo.attachmentCount = 1;
// 	_ResultFramebufferInfo.pAttachments = &resultImageView;
// 	_ResultFramebufferInfo.width = _resultImageCreateInfo.extent.width;
// 	_ResultFramebufferInfo.height = _resultImageCreateInfo.extent.height;
// 	_ResultFramebufferInfo.layers = 1;
// 	if (vkCreateFramebuffer(SmoothieCore::getDevice(), &_ResultFramebufferInfo, nullptr, &resultFramebuffer) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create result framebuffer!" << std::endl;
// 		return 1;
// 	}
//
//
// 	// resultDescriptorSet_ImGuiTexture = ImGui_ImplVulkan_AddTexture(
// 	// 	getSampler("Texture2DModelSampler"),
// 	// 	resultImageView,
// 	// 	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
// 	// );
//
//
// 	VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
// 	vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
// 	vertexShaderPipelineCreateInfo.pName = "main";
// 	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
// 	//vertexShaderPipelineCreateInfo.module = getSystemShaderModule("SYSTEM");
//
// 	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
// 	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
// 	fragmentShaderPipelineCreateInfo.pName = "main";
// 	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
// 	//fragmentShaderPipelineCreateInfo.module = getSystemShaderModule("PRESENT");
//
// 	const VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
// 	VkGraphicsPipelineCreateInfo pipelineInfo{};
// 	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
// 	pipelineInfo.stageCount = 2;
// 	pipelineInfo.pStages = stages;
// 	// Smoothie::DefaultPipelineState __state;
// 	// __state.populate_pipeline(pipelineInfo);
// 	pipelineInfo.renderPass = resultRenderPass;
// 	//pipelineInfo.layout = presentPipelineLayout;
// 	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), nullptr, 1, &pipelineInfo, nullptr, &resultingPipeline) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create resulting pipeline!" << std::endl;
// 		return 1;
// 	}
//
//
// 	swizzler.shader = getEditorShader("SWIZZLER");
// 	//swizzler.sampler = getSampler("Texture2DModelSampler");
// 	if (swizzler.create() != 0)
// 	{
// 		//std::cout << "Failed to create swizzler pass!" << std::endl;
// 		return 1;
// 	}
// 	swizzler.update_destination_image(resultImage, resultImageView);
//
// 	data_getter.shader = getEditorShader("GPU_SELECTED_DATA");
// 	//data_getter.cameraDescriptorSetLayout = getDescriptorSetLayout(0);
// 	//data_getter.depthImage = hdrPass.gDepth.getImage();
// 	//data_getter.depthImageView = hdrPass.gDepth.getImageView();
// 	//data_getter.sampler = getSampler("Texture2DModelSampler");
// 	if (data_getter.create() != 0)
// 	{
// 		//std::cout << "Failed to create data getter from GPU data!" << std::endl;
// 		return 1;
// 	}
//
// 	return 0;
// }
//
// void SmoothieEditor::RenderingManager::destroy_imgui_data()
// {
// 	data_getter.destroy();
// 	swizzler.destroy();
// 	vkDestroyPipeline(SmoothieCore::getDevice(), resultingPipeline, nullptr), resultingPipeline = nullptr;
// 	ImGui_ImplVulkan_RemoveTexture(resultDescriptorSet_ImGuiTexture);
// 	vkDestroyFramebuffer(SmoothieCore::getDevice(), resultFramebuffer, nullptr), resultFramebuffer = nullptr;
// 	vkDestroyImageView(SmoothieCore::getDevice(), resultImageView, nullptr), resultImageView = nullptr;
// 	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), resultImage, resultImageAllocation);
// 	resultImage, resultImageAllocation = nullptr;
// 	vkDestroyRenderPass(SmoothieCore::getDevice(), resultRenderPass, nullptr), resultRenderPass = nullptr;
//
// 	for (auto& [key, value]: editor_shaders)
// 	{
// 		vkDestroyShaderModule(SmoothieCore::getDevice(), value, nullptr), value = nullptr;
// 	}
// 	editor_shaders.clear();
// }
//
// void SmoothieEditor::RenderingManager::draw(VkCommandBuffer commandBuffer, unsigned int currentFrame) const
// {
// 	//const VkDescriptorSet descriptorSet = getDescriptorSet();
// 	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0,
// 		0, nullptr,
// 		0, nullptr,
// 		0, nullptr);
// 	//gBuffer_Pass.bindPass(commandBuffer, currentFrame);
// 	// for (const auto& [key, pipe] : gBufferPipelines)
// 	// {
// 	// 	const auto* _ptr = pipe.get();
// 	// 	if (_ptr != nullptr) _ptr->bindAndDraw(commandBuffer, descriptorSet, currentFrame);
// 	// }
// 	//gBuffer_Pass.unbindPass(commandBuffer, currentFrame);
//
// 	//Screen space lighting effects
// 	//ssao.draw(commandBuffer, descriptorSet, currentFrame);
//
// 	//HDR pass
// 	//hdrPass.bindPass(commandBuffer, currentFrame);
// 	//lighting_global.draw(commandBuffer, descriptorSet, currentFrame);
// 	//skybox.draw(commandBuffer, descriptorSet, currentFrame);
// 	// for (const auto& [key, pipe] : HDRPipelines)
// 	// {
// 	// 	if (pipe != nullptr) pipe->bindAndDraw(commandBuffer, descriptorSet, currentFrame);
// 	// }
// 	//hdrPass.unbindPass(commandBuffer, currentFrame);
//
// 	//Post-processing effects
// 	//bloom.draw(commandBuffer, descriptorSet, currentFrame);
//
//
// 	VkRenderPassBeginInfo beginInfo{};
// 	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
// 	beginInfo.pNext = nullptr;
// 	VkClearValue clearValue{};
// 	clearValue.depthStencil = { 0.0f, 0 };
// 	clearValue.color.float32[0] = 0.0f;
// 	clearValue.color.float32[1] = 0.0f;
// 	clearValue.color.float32[2] = 0.0f;
// 	clearValue.color.float32[3] = 1.0f;
// 	beginInfo.clearValueCount = 1;
// 	beginInfo.pClearValues = &clearValue;
// 	beginInfo.renderArea.offset = { 0, 0 };
// 	beginInfo.renderArea.extent.height = SmoothieCore::getScrHeight();
// 	beginInfo.renderArea.extent.width = SmoothieCore::getScrWidth();
// 	beginInfo.renderPass = resultRenderPass;
// 	beginInfo.framebuffer = resultFramebuffer;
// 	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
// 	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, resultingPipeline);
// 	SmoothieCore::setViewport(commandBuffer);
// 	SmoothieCore::setScissor(commandBuffer);
// 	// const VkDescriptorSet _descriptors[3] =
// 	// {
// 	// 	SmoothieCore::getCameraDescriptorSet(),
// 	// 	descriptorSet,
// 	// 	presentDescriptorSet
// 	// };
// 	//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, presentPipelineLayout, 0, 3, _descriptors, 0, 0);
// 	vkCmdDraw(commandBuffer, 6, 1, 0, 0);
//
// 	vkCmdEndRenderPass(commandBuffer);
// 	VkMemoryBarrier memoryBarrier = {};
// 	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
// 	memoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
// 	memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
// 	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
// 		1, &memoryBarrier,
// 		0, nullptr,
// 		0, nullptr);
//
// 	//SmoothieEditor::Core::getCore().on_command_record_time(commandBuffer, resultDescriptorSet_ImGuiTexture, gBuffer_Pass.gDepth.getImageView());
//
// }
//
// int SmoothieEditor::RenderingManager::resize_callback()
// {
// 	vkQueueWaitIdle(SmoothieCore::getGraphicsQueue());
// 	vkDeviceWaitIdle(SmoothieCore::getDevice());
// 	Smoothie::DeferredRendering::Drawing::resize_callback();
//
// 	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), resultImage, resultImageAllocation);
// 	resultImage, resultImageAllocation = nullptr;
// 	VkImageCreateInfo _resultImageCreateInfo{};
// 	_resultImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
// 	_resultImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
// 	_resultImageCreateInfo.extent.width = SmoothieCore::getScrWidth();
// 	_resultImageCreateInfo.extent.height = SmoothieCore::getScrHeight();
// 	_resultImageCreateInfo.extent.depth = 1;
// 	_resultImageCreateInfo.mipLevels = 1;
// 	_resultImageCreateInfo.arrayLayers = 1;
// 	_resultImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
// 	_resultImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 	_resultImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
// 	_resultImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
// 	_resultImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
// 	_resultImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
// 	VmaAllocationCreateInfo vmaImageAllocationInfo{};
// 	vmaImageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
// 	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_resultImageCreateInfo, &vmaImageAllocationInfo, &resultImage, &resultImageAllocation, nullptr) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create result image!" << std::endl;
// 		return 1;
// 	}
//
// 	vkDestroyImageView(SmoothieCore::getDevice(), resultImageView, nullptr), resultImageView = nullptr;
// 	VkImageViewCreateInfo _resultImageViewCreateInfo{};
// 	_resultImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
// 	_resultImageViewCreateInfo.image = resultImage;
// 	_resultImageViewCreateInfo.format = _resultImageCreateInfo.format;
// 	_resultImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
// 	_resultImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 	_resultImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
// 	_resultImageViewCreateInfo.subresourceRange.levelCount = 1;
// 	_resultImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
// 	_resultImageViewCreateInfo.subresourceRange.layerCount = 1;
// 	if (vkCreateImageView(SmoothieCore::getDevice(), &_resultImageViewCreateInfo, nullptr, &resultImageView) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create Image view for result!" << std::endl;
// 		return 1;
// 	}
//
// 	vkDestroyFramebuffer(SmoothieCore::getDevice(), resultFramebuffer, nullptr), resultFramebuffer = nullptr;
// 	VkFramebufferCreateInfo _ResultFramebufferInfo{};
// 	_ResultFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
// 	_ResultFramebufferInfo.renderPass = resultRenderPass;
// 	_ResultFramebufferInfo.attachmentCount = 1;
// 	_ResultFramebufferInfo.pAttachments = &resultImageView;
// 	_ResultFramebufferInfo.width = _resultImageCreateInfo.extent.width;
// 	_ResultFramebufferInfo.height = _resultImageCreateInfo.extent.height;
// 	_ResultFramebufferInfo.layers = 1;
// 	if (vkCreateFramebuffer(SmoothieCore::getDevice(), &_ResultFramebufferInfo, nullptr, &resultFramebuffer) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create result framebuffer!" << std::endl;
// 		return 1;
// 	}
//
// 	ImGui_ImplVulkan_RemoveTexture(resultDescriptorSet_ImGuiTexture);
// 	resultDescriptorSet_ImGuiTexture = nullptr;
// 	// resultDescriptorSet_ImGuiTexture = ImGui_ImplVulkan_AddTexture(
// 	// 	getSampler("Texture2DModelSampler"),
// 	// 	resultImageView,
// 	// 	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
// 	// );
//
// 	swizzler.update_destination_image(resultImage, resultImageView);
//
// 	//update_swizzler(getCore().editor_window.camera_window.__selected_pass);
// 	//data_getter.update_depth_image(hdrPass.gDepth.getImage(), hdrPass.gDepth.getImageView());
// 	return 0;
// }
//
// VkShaderModule SmoothieEditor::RenderingManager::getEditorShader(const std::string& name) const
// {
// 	if (editor_shaders.find(name) != editor_shaders.end())
// 	{
// 		return editor_shaders.at(name);
// 	}
// 	return nullptr;
// }
//
// void SmoothieEditor::RenderingManager::update_swizzler(int index)
// {
// 	vkDeviceWaitIdle(SmoothieCore::getDevice());
// 	VkImage __scrImage = nullptr;
// 	VkImageView __scrImageView = nullptr;
// 	// SmoothieMath::Matrix4x4 mat;
// 	//
// 	// mat.column0.x = 0.0f;
// 	// mat.column1.y = 0.0f;
// 	// mat.column2.z = 0.0f;
// 	// switch (index)
// 	// {
// 	// case 1: //Albedo
// 	// 	__scrImageView = gBuffer_Pass.gAlbedo.getImageView();
// 	// 	mat.column0.x = 1.0f;
// 	// 	mat.column1.y = 1.0f;
// 	// 	mat.column2.z = 1.0f;
// 	// 	break;
// 	//
// 	// case 2: //Metalic
// 	// 	__scrImageView = gBuffer_Pass.gMRAO.getImageView();
// 	// 	mat.column0.x = 1.0f;
// 	// 	mat.column0.y = 1.0f;
// 	// 	mat.column0.z = 1.0f;
// 	// 	break;
// 	//
// 	// case 3: //Roughness
// 	// 	__scrImageView = gBuffer_Pass.gMRAO.getImageView();
// 	// 	mat.column1.x = 1.0f;
// 	// 	mat.column1.y = 1.0f;
// 	// 	mat.column1.z = 1.0f;
// 	// 	break;
// 	//
// 	// case 4: //Baked AO
// 	// 	__scrImageView = gBuffer_Pass.gMRAO.getImageView();
// 	// 	mat.column3.x = 1.0f;
// 	// 	mat.column3.y = 1.0f;
// 	// 	mat.column3.z = 1.0f;
// 	// 	break;
// 	//
// 	// case 5: //Normal
// 	// 	__scrImageView = gBuffer_Pass.gNormal.getImageView();
// 	// 	mat.column0.x = 1.0f;
// 	// 	mat.column1.y = 1.0f;
// 	// 	mat.column2.z = 1.0f;
// 	// 	break;
// 	//
// 	// case 6: //Depth
// 	// 	__scrImageView = gBuffer_Pass.gDepth.getImageView();
// 	// 	mat.column0.x = 1.0f;
// 	// 	mat.column0.y = 1.0f;
// 	// 	mat.column0.z = 1.0f;
// 	// 	break;
// 	//
// 	// default:
// 	// 	break;
// 	// }
//
// 	if (__scrImageView == nullptr)
// 	{
// 		__scrImageView = resultImageView;
// 	}
//
// 	swizzler.update_source_image(__scrImage, __scrImageView);
// 	//swizzler.update_matrix(mat);
// }
//
//
//
//
//
// int SmoothieEditor::DeviceDataGetter::create()
// {
// 	//************************* Create descriptor set layout ***************************//
// 	VkDescriptorSetLayoutBinding __resultBufferSetBinding = {};
// 	__resultBufferSetBinding.binding = 0;
// 	__resultBufferSetBinding.descriptorCount = 1;
// 	__resultBufferSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
// 	__resultBufferSetBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
//
// 	VkDescriptorSetLayoutBinding __depthImageSetBinding = {};
// 	__depthImageSetBinding.binding = 1;
// 	__depthImageSetBinding.descriptorCount = 1;
// 	__depthImageSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// 	__depthImageSetBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
//
//
// 	const VkDescriptorSetLayoutBinding __descriptorSetBindings[] = { __resultBufferSetBinding, __depthImageSetBinding };
// 	VkDescriptorSetLayoutCreateInfo __setLayoutCreateInfo = {};
// 	__setLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
// 	__setLayoutCreateInfo.bindingCount = 2;
// 	__setLayoutCreateInfo.pBindings = __descriptorSetBindings;
// 	if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &__setLayoutCreateInfo, nullptr, &bufferDescriptorSetLayout) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create descriptor set layout!" << std::endl;
// 		return 1;
// 	}
//
// 	for (auto& __descriptor: descriptors)
// 	{
// 		//************************* Create descriptor pool ***************************//
// 		VkDescriptorPoolSize __depthImagePoolSize = {};
// 		__depthImagePoolSize.descriptorCount = 1;
// 		__depthImagePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//
// 		VkDescriptorPoolSize __resultBufferPoolSize = {};
// 		__resultBufferPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
// 		__resultBufferPoolSize.descriptorCount = 1;
//
// 		const VkDescriptorPoolSize __poolSizes[] = { __depthImagePoolSize, __resultBufferPoolSize };
// 		VkDescriptorPoolCreateInfo __poolCreateInfo = {};
// 		__poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
// 		__poolCreateInfo.maxSets = 1;
// 		__poolCreateInfo.poolSizeCount = 2;
// 		__poolCreateInfo.pPoolSizes = __poolSizes;
// 		if (vkCreateDescriptorPool(SmoothieCore::getDevice(), &__poolCreateInfo, nullptr, &__descriptor.descriptorPool) != VK_SUCCESS)
// 		{
// 			//std::cout << "Failed to create descriptor pool!" << std::endl;
// 			return 1;
// 		}
//
// 		//************************* Allocate descriptor sets ***************************//
// 		VkDescriptorSetAllocateInfo __allocInfo = {};
// 		__allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
// 		__allocInfo.descriptorPool = __descriptor.descriptorPool;
// 		__allocInfo.descriptorSetCount = 1;
// 		__allocInfo.pSetLayouts = &bufferDescriptorSetLayout;
// 		if (vkAllocateDescriptorSets(SmoothieCore::getDevice(), &__allocInfo, &__descriptor.descriptorSet) != VK_SUCCESS)
// 		{
// 			//std::cout << "Failed to allocate descriptor set!" << std::endl;
// 			return 1;
// 		}
//
// 		//************************* Create result buffer *****************************//
//
// 		VkBufferCreateInfo __bufferCreateInfo = {};
// 		__bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
// 		__bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
// 		__bufferCreateInfo.size = sizeof(_Descriptors);
// 		__bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//
// 		VmaAllocationCreateInfo __alocationCreateInfo = {};
// 		__alocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
// 		if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &__bufferCreateInfo, &__alocationCreateInfo, &__descriptor.buffer, &__descriptor.allocation, nullptr) != VK_SUCCESS)
// 		{
// 			//std::cout << "Failed to create buffer!" << std::endl;
// 			return 1;
// 		}
//
// 		//************************* Update descriptor sets ***************************//
//
// 		VkDescriptorBufferInfo __bufferInfo = {};
// 		__bufferInfo.buffer = __descriptor.buffer;
// 		__bufferInfo.offset = 0;
// 		__bufferInfo.range = VK_WHOLE_SIZE;
//
//
// 		VkWriteDescriptorSet descriptorWrite{};
// 		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// 		descriptorWrite.dstSet = __descriptor.descriptorSet;
// 		descriptorWrite.dstArrayElement = 0;
// 		descriptorWrite.dstBinding = 0;
// 		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
// 		descriptorWrite.descriptorCount = 1;
// 		descriptorWrite.pBufferInfo = &__bufferInfo;
// 		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
//
// 		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// 		descriptorWrite.pBufferInfo = nullptr;
//
// 		VkDescriptorImageInfo imgInfo{};
// 		imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 		imgInfo.imageView = depthImageView;
// 		imgInfo.sampler = sampler;
//
// 		descriptorWrite.dstBinding = 1;
// 		descriptorWrite.pImageInfo = &imgInfo;
// 		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
// 	}
//
//
// 	VkPushConstantRange __push_constant = {};
// 	__push_constant.offset = 0;
// 	__push_constant.size = sizeof(ImVec2);
// 	__push_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
//
// 	VkPipelineLayoutCreateInfo __pipelineLayoutCreateInfo = {};
// 	__pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
// 	__pipelineLayoutCreateInfo.setLayoutCount = 2;
// 	const VkDescriptorSetLayout __descriptorSetLayouts[] = {cameraDescriptorSetLayout, bufferDescriptorSetLayout};
// 	__pipelineLayoutCreateInfo.pSetLayouts = __descriptorSetLayouts;
// 	__pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
// 	__pipelineLayoutCreateInfo.pPushConstantRanges = &__push_constant;
// 	if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &__pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create pipeline layout!" << std::endl;
// 		return 1;
// 	}
//
// 	VkPipelineShaderStageCreateInfo __shaderCreateInfo = {};
// 	__shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
// 	__shaderCreateInfo.pName = "main";
// 	__shaderCreateInfo.module = shader;
// 	__shaderCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
//
// 	VkComputePipelineCreateInfo __pipelineCreateInfo = {};
// 	__pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
// 	__pipelineCreateInfo.layout = pipelineLayout;
// 	__pipelineCreateInfo.stage = __shaderCreateInfo;
// 	if (vkCreateComputePipelines(SmoothieCore::getDevice(), nullptr, 1, &__pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create pipeline!" << std::endl;
// 		return 1;
// 	}
//
// 	return 0;
// }
//
// void SmoothieEditor::DeviceDataGetter::compute(VkCommandBuffer commandBuffer, const ImVec2& uv) const
// {
// 	VkMemoryBarrier __barrier = {};
// 	__barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
// 	__barrier.srcAccessMask = VK_ACCESS_HOST_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
// 	__barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
// 	vkCmdPipelineBarrier(commandBuffer,
// 		VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
// 		1, &__barrier,
// 		0, nullptr,
// 		0, nullptr);
//
// 	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
// 	// const VkDescriptorSet __sets[] =
// 	// {
// 	// 	SmoothieCore::getCameraDescriptorSet(),
// 	// 	descriptors[SmoothieCore::getCurrentFrame()].descriptorSet
// 	// };
// 	//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 2, __sets, 0, nullptr);
// 	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ImVec2), &uv);
// 	vkCmdDispatch(commandBuffer, 1, 1, 1);
//
// 	__barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
// 	__barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
// 	vkCmdPipelineBarrier(commandBuffer,
// 		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,  0,
// 		1, &__barrier,
// 		0, nullptr,
// 		0, nullptr);
// }
//
// void SmoothieEditor::DeviceDataGetter::destroy()
// {
// 	vkDestroyPipeline(SmoothieCore::getDevice(), pipeline, nullptr), pipeline = nullptr;
// 	vkDestroyPipelineLayout(SmoothieCore::getDevice(), pipelineLayout, nullptr), pipelineLayout = nullptr;
// 	for (auto& __descriptor : descriptors)
// 	{
// 		vkDestroyDescriptorPool(SmoothieCore::getDevice(), __descriptor.descriptorPool, nullptr);
// 		__descriptor.descriptorPool = nullptr, __descriptor.descriptorSet = nullptr;
// 		vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), __descriptor.buffer, __descriptor.allocation);
// 		__descriptor.buffer = nullptr, __descriptor.allocation = nullptr;
// 	}
// 	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), bufferDescriptorSetLayout, nullptr);
// 	bufferDescriptorSetLayout = nullptr;
// }
//
// int SmoothieEditor::DeviceDataGetter::get_current_data(Data& data) const
// {
// 	const auto& _d = descriptors[SmoothieCore::getCurrentFrame()];
// 	vmaCopyAllocationToMemory(SmoothieCore::getVulkanMemoryAllocator(), _d.allocation, 0, &data, sizeof(Data));
// 	return 0;
// }
//
// void SmoothieEditor::DeviceDataGetter::update_depth_image(VkImage image, VkImageView view)
// {
// 	vkQueueWaitIdle(SmoothieCore::getGraphicsQueue());
// 	depthImage = image;
// 	depthImageView = view;
// 	for (auto& __descriptor : descriptors)
// 	{
// 		VkDescriptorImageInfo imgInfo{};
// 		imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 		imgInfo.imageView = view;
// 		imgInfo.sampler = sampler;
//
// 		VkWriteDescriptorSet descriptorWrite{};
// 		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// 		descriptorWrite.dstSet = __descriptor.descriptorSet;
// 		descriptorWrite.dstArrayElement = 0;
// 		descriptorWrite.dstBinding = 1;
// 		descriptorWrite.descriptorCount = 1;
// 		descriptorWrite.pBufferInfo = nullptr;
// 		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// 		descriptorWrite.pImageInfo = &imgInfo;
// 		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
// 	}
// }
