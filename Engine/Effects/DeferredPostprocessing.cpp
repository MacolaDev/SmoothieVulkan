#include "DeferredPostprocessing.h"
#include <array>
#define _SMOOTHIE_ENGINE
#include "Core/SmoothieCore.h"
#include <vector>
#include "Core/UniformBuffers.h"
#include "Core/PostProcessing.h"
#include "Core/Shader.h"
#include "Core/SwapChain.h"
#include "Core/CameraDescriptor.h"

VkPipeline DeferredPostprocessing::pipeline = nullptr;
VkPipelineLayout DeferredPostprocessing::pipelineLayout = nullptr;

VkDescriptorPool DeferredPostprocessing::descriptorPool = nullptr;
VkDescriptorSetLayout DeferredPostprocessing::descriptorSetLayout = nullptr;
VkDescriptorSet DeferredPostprocessing::descriptorSet = nullptr;

void DeferredPostprocessing::create(const Image& HDRImage, const Image& bloomImage)
{
	std::array<VkImageView, 2> images =
	{
		HDRImage.imageView, bloomImage.imageView
	};
	
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	std::array<VkDescriptorPoolSize, images.size()> poolSizes = { poolSize ,poolSize };
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<unsigned int>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1;
	vkCreateDescriptorPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &descriptorPool);

	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
	for (int i = 0; i < images.size(); i++) 
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = i;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBindings.push_back(layoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<unsigned int>(layoutBindings.size());
	layoutInfo.pBindings = layoutBindings.data();
	vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &layoutInfo, nullptr, &descriptorSetLayout);

	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
	vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &descriptorSet);

	for (int i = 0; i < images.size(); i++) 
	{
		VkDescriptorImageInfo imgInfo{};
		imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgInfo.imageView = images[i];
		imgInfo.sampler = Samplers::ClampToEdgeLINEAR;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = i;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imgInfo;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	//Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	VkDescriptorSetLayout descriptorSetLayouts[] =
	{
		CameraDescriptor::descriptorSetLayout,
		StandardUnifromBuffers::buffer2.getDescriptorSetLayout(),
		descriptorSetLayout
	};

	pipelineLayoutInfo.setLayoutCount = 3;
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
	vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);



	//Pipeline
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
	fragmentShaderPipelineCreateInfo.module = PostProcessingShaders::DeferredPipelineFinal;

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
	pipelineInfo.renderPass = SwapChain::getDefaultRenderPass();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;

	vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
}

void DeferredPostprocessing::destroy()
{
	vkDestroyPipeline(SmoothieCore::getDevice(), pipeline, nullptr);
	pipeline = nullptr;

	vkDestroyPipelineLayout(SmoothieCore::getDevice(), pipelineLayout, nullptr);
	pipelineLayout = nullptr;

	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), descriptorSetLayout, nullptr);
	descriptorSetLayout = nullptr;

	vkDestroyDescriptorPool(SmoothieCore::getDevice(), descriptorPool, nullptr);
	descriptorPool = nullptr;
}

void DeferredPostprocessing::update(const Image& HDRImage, const Image& bloomImage)
{
	destroy();
	create(HDRImage, bloomImage);
}

void DeferredPostprocessing::draw(VkCommandBuffer commandBuffer, int index)
{
	//Begin bass
	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.framebuffer = SwapChain::getSwapChainFramebuffer(index);
	beginInfo.renderPass = SwapChain::getDefaultRenderPass();

	VkClearValue clearColor{};
	clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	beginInfo.clearValueCount = 1;
	beginInfo.pClearValues = &clearColor;
	beginInfo.renderArea.extent = SwapChain::getSwapChainExtent();
	beginInfo.renderArea.offset = { 0, 0 };

	SmoothieCore::setViewport(commandBuffer);

	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	

	SmoothieCore::setViewport(commandBuffer);
	SmoothieCore::setScissor(commandBuffer);

	VkDescriptorSet descriptorSets[] =
	{
		CameraDescriptor::descriptorSet,
		StandardUnifromBuffers::buffer2.getDescriptorSet(),
		descriptorSet
	};

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,

		pipelineLayout, 0,
		3, descriptorSets,
		0, nullptr);
	vkCmdDraw(commandBuffer, 6, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);
}
