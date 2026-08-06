#include "SSAO.h"

#include <array>
#include <random>

#include "Core/SmoothieCore.h"
#include "Core/Shader.h"
#include "Effects/Deferred_Core.h"

using namespace Smoothie;

static inline float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

// static void generateNoise(
// 	std::vector<SmoothieMath::Vector4>& ssaoKernel,
// 	std::vector<SmoothieMath::Vector4>& ssaoNoise
// 	)
// {
// 	std::uniform_real_distribution<float> randomFloats(-1.0f, 1.0f);
// 	std::uniform_real_distribution<float> randomFloats01(0.0f, 1.0f);
// 	std::default_random_engine generator;
//
// 	for (size_t i = 0; i < ssaoKernel.size(); ++i)
// 	{
// 		Vector4 sample(
// 			randomFloats(generator),
// 			randomFloats(generator),
// 			randomFloats(generator), 0.0f
// 		);
// 		sample.normalizeVector();
// 		sample = sample * randomFloats01(generator);
//
// 		float scale = (float)i / 64.0f;
// 		scale = lerp(0.1f, 1.0f, scale * scale);
// 		sample = sample * scale;
//
// 		ssaoKernel[i] = sample;
// 	}
//
// 	for (size_t i = 0; i < ssaoNoise.size(); i++)
// 	{
// 		Vector4 noise = Vector4(randomFloats(generator), randomFloats(generator), 0.0f, 0.0f);
// 		noise.normalizeVector();
// 		ssaoNoise[i] = noise;
// 	}
//
// }

//static void generateNoiseTexture(Image& image, const std::vector<Vector4>& ssaoNoise) 
//{
//	
//	const auto imageSize = static_cast<unsigned int>(sqrt(ssaoNoise.size()));
//
//	VkImageCreateInfo createImage{};
//	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//	createImage.imageType = VK_IMAGE_TYPE_2D;
//	createImage.extent.width = imageSize;
//	createImage.extent.height = imageSize;
//	createImage.extent.depth = 1;
//	createImage.mipLevels = 1;
//	createImage.arrayLayers = 1;
//	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
//	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//	createImage.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
//	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//	createImage.format = VK_FORMAT_R32G32B32A32_SFLOAT;
//
//	//Creating image
//	VmaAllocationCreateInfo vmaImageAllocationInfo{};
//	vmaImageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
//	vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &createImage, &vmaImageAllocationInfo, &image.image, &image.allocation, nullptr);
//
//
//	std::string imageAllocationName = "Texture: SSAO noise";
//	vmaSetAllocationName(SmoothieCore::getVulkanMemoryAllocator(), image.allocation, imageAllocationName.c_str());
//
//
//	VkBuffer stagingBuffer = nullptr;
//	VmaAllocation stagingBufferAllocation = nullptr;
//
//	VmaAllocationCreateInfo stagingBufferAllocInfo = {};
//	stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
//	stagingBufferAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
//	stagingBufferAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
//
//	VkBufferCreateInfo stagingBufferCreateInfo{};
//	stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	stagingBufferCreateInfo.size = ssaoNoise.size() * sizeof(Vector4);
//	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//
//	vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &stagingBufferCreateInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingBufferAllocation, nullptr);
//
//
//	std::string stagingBufferAllocationName = "Texture stageing buffer: SSAO noise texture";
//	vmaSetAllocationName(SmoothieCore::getVulkanMemoryAllocator(), stagingBufferAllocation, stagingBufferAllocationName.c_str());
//	vmaCopyMemoryToAllocation(SmoothieCore::getVulkanMemoryAllocator(), ssaoNoise.data(), stagingBufferAllocation, 0, stagingBufferCreateInfo.size);
//
//	transitionImageLayout(image.image, createImage.format, createImage.initialLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
//	copyBufferToImage(stagingBuffer, image.image, imageSize, imageSize);
//	transitionImageLayout(image.image, createImage.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
//	
//	vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), stagingBuffer, stagingBufferAllocation);
//	
//	VkImageViewCreateInfo imageViewInfo{};
//	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//	imageViewInfo.image = image.image;
//	imageViewInfo.format = createImage.format;
//	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//	imageViewInfo.subresourceRange.baseMipLevel = 0;
//	imageViewInfo.subresourceRange.levelCount = 1;
//	imageViewInfo.subresourceRange.baseArrayLayer = 0;
//	imageViewInfo.subresourceRange.layerCount = 1;
//	vkCreateImageView(SmoothieCore::getDevice(), &imageViewInfo, nullptr, &image.imageView);
//
//}

//static void generateNoiseKernelUBO(VmaAllocation& allocation, VkBuffer& buffer, const std::vector<Vector4>& kernel) 
//{
//	
//	VkBufferCreateInfo bufferCreateInfo{};
//	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	bufferCreateInfo.size = kernel.size() * sizeof(Vector4);
//	bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
//
//	VmaAllocationCreateInfo bufferAllocationInfo{};
//	bufferAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
//	bufferAllocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
//
//	vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &bufferCreateInfo, &bufferAllocationInfo,
//		&buffer,
//		&allocation,
//		nullptr);
//
//	vmaCopyMemoryToAllocation(
//		SmoothieCore::getVulkanMemoryAllocator(),
//		kernel.data(),
//		allocation, 0,
//		kernel.size() * sizeof(Vector4));
//
//}

//static void createImageAndFramebuffer(unsigned int texWidth, unsigned int texHeight, VkFramebuffer& framebuffer, Image& renderTarget, const VkRenderPass& renderPass)
//{
//	VkImageCreateInfo createImage{};
//	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//	createImage.imageType = VK_IMAGE_TYPE_2D;
//	createImage.extent = { texWidth , texHeight , 1 };
//	createImage.mipLevels = 1;
//	createImage.arrayLayers = 1;
//	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
//	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//	createImage.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
//	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//	createImage.format = VK_FORMAT_R8G8B8A8_UNORM;
//
//
//	VmaAllocationCreateInfo allocInfo = {};
//	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
//	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
//	allocInfo.priority = 1.0f;
//
//	vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &createImage, &allocInfo, &renderTarget.image, &renderTarget.allocation, &renderTarget.allocationInfo);
//
//	VkImageViewCreateInfo createImageView{};
//	createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//	createImageView.image = renderTarget.image;
//	createImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
//	createImageView.format = VK_FORMAT_R8G8B8A8_UNORM;
//	createImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//	createImageView.subresourceRange.levelCount = 1;
//	createImageView.subresourceRange.layerCount = 1;
//	vkCreateImageView(SmoothieCore::getDevice(), &createImageView, nullptr, &renderTarget.imageView);
//
//	VkFramebufferCreateInfo createInfo{};
//	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//	createInfo.renderPass = renderPass;
//	createInfo.attachmentCount = 1;
//	createInfo.pAttachments = &renderTarget.imageView;
//	createInfo.width = texWidth;
//	createInfo.height = texHeight;
//	createInfo.layers = 1;
//	vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &framebuffer);
//
//}

//void SSAO::create(unsigned int width, unsigned int height, const Image& gDepth, const Image& gNormal)
//{
//	ssaoNoise.resize(16);
//	ssaoKernel.resize(64);
//	generateNoise(ssaoKernel, ssaoNoise);
//	generateNoiseTexture(noiseTexture, ssaoNoise);
//	generateNoiseKernelUBO(kernelUBOAllocation, kernelUBO, ssaoKernel);
//
//	createImageAndFramebuffer(width / 2, height / 2, framebuffer, SSAOImage, RenderPass::SSAO);
//
//
//	//Descriptor pool
//	VkDescriptorPoolSize texturePoolSize{};
//	texturePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//	texturePoolSize.descriptorCount = 3; //gPosition, gNormal, noiseTexture
//
//	VkDescriptorPoolSize UBOPoolSize{};
//	UBOPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//	UBOPoolSize.descriptorCount = 1; //UBO kernels
//
//	VkDescriptorPoolCreateInfo poolInfo{};
//	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//	
//	VkDescriptorPoolSize sizes[] = { texturePoolSize , UBOPoolSize };
//	poolInfo.poolSizeCount = 1;
//	poolInfo.pPoolSizes = sizes;
//	poolInfo.maxSets = 1;
//	vkCreateDescriptorPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &descriptorPool);
//
//
//	//Descriptor bindings
//	std::array<VkDescriptorSetLayoutBinding, 4> bindings{};
//	for (unsigned int i = 0; i < bindings.size() - 1; i++)
//	{
//		VkDescriptorSetLayoutBinding layoutBinding{};
//		layoutBinding.binding = i;
//		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//		layoutBinding.descriptorCount = 1;
//		layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
//		layoutBinding.pImmutableSamplers = nullptr;
//
//		bindings[i] = layoutBinding;
//	}
//
//	VkDescriptorSetLayoutBinding layoutBinding{};
//	layoutBinding.binding = 3;
//	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//	layoutBinding.descriptorCount = 1;
//	layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
//	layoutBinding.pImmutableSamplers = nullptr;
//	bindings[3] = layoutBinding;
//
//	//Descriptor set layout
//	VkDescriptorSetLayoutCreateInfo layoutInfo{};
//	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//	layoutInfo.bindingCount = static_cast<unsigned int>(bindings.size());
//	layoutInfo.pBindings = bindings.data();
//	vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &layoutInfo, nullptr, &descriptorSetLayout);
//
//	//Allocate and update descriptor sets
//	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
//	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//	descriptorSetAllocInfo.descriptorPool = descriptorPool;
//	descriptorSetAllocInfo.descriptorSetCount = 1;
//	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
//	vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &descriptorSet);
//
//	const VkImageView images[] = 
//	{
//		gDepth.imageView,
//		gNormal.imageView,
//		noiseTexture.imageView
//	};
//
//	for (unsigned int i = 0; i < 3; i++)
//	{
//	
//		VkDescriptorImageInfo imgInfo{};
//		imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//		imgInfo.imageView = images[i];
//		imgInfo.sampler = Samplers::RepeatNEAREST;
//
//		VkWriteDescriptorSet descriptorWrite{};
//		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//		descriptorWrite.dstSet = descriptorSet;
//		descriptorWrite.dstArrayElement = 0;
//		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//		descriptorWrite.descriptorCount = 1;
//		descriptorWrite.dstBinding = i;
//		descriptorWrite.pBufferInfo = nullptr;
//		descriptorWrite.pImageInfo = &imgInfo;
//		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
//	}
//
//	VkDescriptorBufferInfo bufferInfo{};
//	bufferInfo.buffer = kernelUBO;
//	bufferInfo.offset = 0;
//	bufferInfo.range = VK_WHOLE_SIZE;
//
//	VkWriteDescriptorSet descriptorWrite{};
//	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//	descriptorWrite.dstSet = descriptorSet;
//	descriptorWrite.dstArrayElement = 0;
//	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//	descriptorWrite.descriptorCount = 1;
//	descriptorWrite.dstBinding = 3;
//	descriptorWrite.pBufferInfo = &bufferInfo;
//	descriptorWrite.pImageInfo = nullptr;
//	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
//
//
//
//
//	//Pipeline layout
//	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
//	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//	VkDescriptorSetLayout descriptorSets[] =
//	{
//		SmoothieCore::getCameraDescriptorSetLayout(0),
//		DeferredRendering::getDrawingCore().getDescriptorSetLayout(0),
//		descriptorSetLayout
//	};
//
//	pipelineLayoutInfo.setLayoutCount = 3;
//	pipelineLayoutInfo.pSetLayouts = descriptorSets;
//	vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
//
//
//	//Pipeline 
//	VkGraphicsPipelineCreateInfo pipelineInfo{};
//	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//
//	VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
//	vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	vertexShaderPipelineCreateInfo.pName = "main";
//	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//	vertexShaderPipelineCreateInfo.module = SmoothieCore::getSystemShaderModule("SYSTEM");
//
//	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
//	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	fragmentShaderPipelineCreateInfo.pName = "main";
//	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//	fragmentShaderPipelineCreateInfo.module = SmoothieCore::getSystemShaderModule("SSAO");
//
//	VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
//	pipelineInfo.stageCount = 2;
//	pipelineInfo.pStages = stages;
//
//	pipelineInfo.pVertexInputState = &PostProcessing::standardVertexState;
//	pipelineInfo.pInputAssemblyState = &PostProcessing::standardAssemblyState;
//	pipelineInfo.pViewportState = &PostProcessing::viewportState;
//	pipelineInfo.pRasterizationState = &PostProcessing::rasterizer;
//	pipelineInfo.pDepthStencilState = &PostProcessing::pipelineDepthStencil;
//	pipelineInfo.pMultisampleState = &PostProcessing::multisampling;
//	pipelineInfo.pColorBlendState = &PostProcessing::colorBlending;
//	pipelineInfo.pDynamicState = &PostProcessing::dynamicState;
//	pipelineInfo.layout = pipelineLayout;
//	pipelineInfo.renderPass = RenderPass::SSAO;
//	pipelineInfo.subpass = 0;
//	pipelineInfo.basePipelineHandle = nullptr;
//
//	vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
//
//}

//void SSAO::draw(VkCommandBuffer commandBuffer, unsigned int currentFrame) const
//{
//	//Render pass begin info
//	VkRenderPassBeginInfo beginInfo{};
//	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//	beginInfo.framebuffer = framebuffer;
//	beginInfo.renderPass = RenderPass::SSAO;
//
//	VkClearValue clearColor{};
//	clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };
//
//	const VkClearValue clearValues[] = { clearColor };
//	beginInfo.clearValueCount = 1;
//	beginInfo.pClearValues = clearValues;
//	beginInfo.renderArea.extent.height = SmoothieCore::getScrHeight() / 2;
//	beginInfo.renderArea.extent.width = SmoothieCore::getScrWidth() / 2;
//	beginInfo.renderArea.offset = { 0, 0 };
//
//	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
//
//	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
//	
//	VkViewport viewport{};
//	viewport.x = 0.0f;
//	viewport.y = 0.0f;
//	viewport.width = static_cast<float>(SmoothieCore::getScrWidth() / 2);
//	viewport.height = static_cast<float>(SmoothieCore::getScrHeight() / 2);
//	viewport.minDepth = 0.0f;
//	viewport.maxDepth = 1.0f;
//	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
//
//	VkRect2D scissor{};
//	scissor.offset = { 0, 0 };
//	scissor.extent.width = SmoothieCore::getScrWidth() / 2;
//	scissor.extent.height = SmoothieCore::getScrHeight() / 2;
//	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
//
//	VkDescriptorSet descriptorSets[] =
//	{
//		SmoothieCore::getCameraDescriptorSet(),
//		DeferredRendering::getDrawingCore().getDescriptorSet(),
//		descriptorSet
//	};
//
//	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
//		pipelineLayout, 0,
//		3, descriptorSets,
//		0, nullptr);
//
//	vkCmdDraw(commandBuffer, 6, 1, 0, 0);
//
//
//	vkCmdEndRenderPass(commandBuffer);
//}

//void SSAO::destroy()
//{
//	vkDestroyPipeline(SmoothieCore::getDevice(), pipeline, nullptr);
//	pipeline = nullptr;
//
//	vkDestroyPipelineLayout(SmoothieCore::getDevice(), pipelineLayout, nullptr);
//	pipelineLayout = nullptr;
//
//	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), descriptorSetLayout, nullptr);
//	descriptorSetLayout = nullptr;
//
//	vkDestroyDescriptorPool(SmoothieCore::getDevice(), descriptorPool, nullptr);
//	descriptorPool = nullptr;
//
//	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr);
//	framebuffer = nullptr;
//	SSAOImage.destroyImage();
//	
//	vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), kernelUBO, kernelUBOAllocation);
//	kernelUBO, kernelUBOAllocation = nullptr;
//	
//	noiseTexture.destroyImage();
//}

int Smoothie::DeferredRendering::SSAO::create()
{
	return 0;
}

int Smoothie::DeferredRendering::SSAO::resize_callback()
{
	return 0;
}

void Smoothie::DeferredRendering::SSAO::draw(VkCommandBuffer commandBuffer, VkDescriptorSet drawClassDescriptor, unsigned int ImageIndex) const
{
}

void Smoothie::DeferredRendering::SSAO::destroy()
{
}
