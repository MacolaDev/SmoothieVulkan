#include "PBR.h"
#include "ResourceManager/ShaderFile.h"
#include "Core/PostProcessing.h"
#define _SMOOTHIE_ENGINE

#include "Core/SmoothieCore.h"
#include "Core/Image.h"
#include <array>

#include "Core/DeferredPipeline.h"
#include "Core/UniformBuffers.h"
#include "Core/Shader.h"
#include "Core/SwapChain.h"
#include "Effects/BRDF.h"
#include "Core/RenderPass.h"
#include "Core/CameraDescriptor.h"

#include <iostream>
#include "Core/VMA.h"

void PBR::create(unsigned int width, unsigned int height, const gBuffer& gBuffrerData, const VkImageView& HDRRenderTarget)
{
	//Framebuffer
	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = PBRPass::renderPass;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &HDRRenderTarget;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = 1;
	vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &framebuffer);

	std::array<VkImageView, 7> images =
	{
		gBuffrerData.gPosition.imageView,
		gBuffrerData.gNormal.imageView,
		gBuffrerData.gAlbedo.imageView,
		gBuffrerData.gMRAO.imageView,
		BRDF::image.imageView,
		PBRCubemaps::IrradianceMap.imageView,
		PBRCubemaps::PrefilterMap.imageView
	};
	
	//Texture descriptor pool
	VkDescriptorPoolSize texturePoolSize{};
	texturePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	texturePoolSize.descriptorCount = static_cast<unsigned int>(images.size());

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &texturePoolSize;

	poolInfo.maxSets = 1;
	vkCreateDescriptorPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &descriptorPool);

	//Texture layouts
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayouts;
	for (int i = 0; i < images.size(); i++)
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = i;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBinding.pImmutableSamplers = nullptr;
		descriptorSetLayouts.push_back(layoutBinding);
	}

	//Descriptor set layout
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<unsigned int>(descriptorSetLayouts.size());
	layoutInfo.pBindings = descriptorSetLayouts.data();
	vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &layoutInfo, nullptr, &descriptorSetLayout);

	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
	vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &descriptorSet);


	//Updating descriptor sets with textures
	for (int i = 0; i < images.size(); i++)
	{
		VkDescriptorImageInfo imgInfo{};
		imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgInfo.imageView = images[i];
		imgInfo.sampler = Samplers::Texture2DModelSampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.dstBinding = i;
		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imgInfo;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	VkDescriptorSetLayout descriptorSets[] =
	{
		CameraDescriptor::descriptorSetLayout,
		StandardUnifromBuffers::buffer2.getDescriptorSetLayout(),
		descriptorSetLayout
	};

	pipelineLayoutInfo.setLayoutCount = 3;
	pipelineLayoutInfo.pSetLayouts = descriptorSets;

	vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);



	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	
	VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
	vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderPipelineCreateInfo.pName = "main";
	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderPipelineCreateInfo.module = PostProcessingShaders::basicVertexShader;

	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderPipelineCreateInfo.pName = "main";
	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderPipelineCreateInfo.module = PostProcessingShaders::PBRShader;
	
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
	pipelineInfo.renderPass = PBRPass::renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;

	vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
}

void PBR::destroy()
{
	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr);
	framebuffer = nullptr;

	vkDestroyPipeline(SmoothieCore::getDevice(), pipeline, nullptr);
	pipeline = nullptr;

	vkDestroyPipelineLayout(SmoothieCore::getDevice(), pipelineLayout, nullptr);
	pipelineLayout = nullptr;

	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), descriptorSetLayout, nullptr);
	descriptorSetLayout = nullptr;

	vkDestroyDescriptorPool(SmoothieCore::getDevice(), descriptorPool, nullptr);
	descriptorPool = nullptr;
}

void PBR::update(unsigned int width, unsigned int height)
{
}

void PBR::draw(VkCommandBuffer commandBuffer) const
{

	//Render pass begin info
	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.framebuffer = framebuffer;
	beginInfo.renderPass = PBRPass::renderPass;

	VkClearValue clearColor{};
	clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };

	static VkClearValue clearValues[] = { clearColor };
	beginInfo.clearValueCount = 1;
	beginInfo.pClearValues = clearValues;

	beginInfo.renderArea.extent = { 1280, 720 };
	beginInfo.renderArea.offset = { 0, 0 };

	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	//Viewport data
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(1280);
	viewport.height = static_cast<float>(720);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = SwapChain::getSwapChainExtent();
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

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


