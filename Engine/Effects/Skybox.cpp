#include "Skybox.h"
#define _SMOOTHIE_ENGINE
#include "Core/SmoothieCore.h"
#include "Core/Shader.h"
#include "Core/UniformBuffers.h"
#include "Core/PostProcessing.h"
#include "Core/RenderPass.h"
#include "Core/CameraDescriptor.h"

VkDescriptorPool Skybox::descriptorPool = nullptr;
VkDescriptorSet Skybox::descriptorSet = nullptr;
VkDescriptorSetLayout Skybox::descriptorSetLayout = nullptr;
VkPipeline Skybox::pipeline = nullptr;
VkPipelineLayout Skybox::pipelineLayout = nullptr;

void Skybox::create(const Image& environmentImage)
{
	//Descriptor pools
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;
	vkCreateDescriptorPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &descriptorPool);

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
	vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &layoutInfo, nullptr, &descriptorSetLayout);

	//Allocating descriptor set
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
	vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &descriptorSet);

	//Updating descriptr set with image
	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgInfo.imageView = environmentImage.imageView;
	imgInfo.sampler = Samplers::Texture2DModelSampler;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pImageInfo = &imgInfo;
	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);

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
	vertexShaderPipelineCreateInfo.module = PostProcessingShaders::SkyboxVertex;

	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderPipelineCreateInfo.pName = "main";
	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderPipelineCreateInfo.module = PostProcessingShaders::SkyboxFragment;

	VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;
	
	VkPipelineDepthStencilStateCreateInfo depthStencil = PostProcessing::pipelineDepthStencil;
	depthStencil.depthTestEnable = true;
	depthStencil.depthWriteEnable = true;

	pipelineInfo.pVertexInputState = &PostProcessing::standardVertexState;
	pipelineInfo.pInputAssemblyState = &PostProcessing::standardAssemblyState;
	pipelineInfo.pViewportState = &PostProcessing::viewportState;
	pipelineInfo.pRasterizationState = &PostProcessing::rasterizer;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pMultisampleState = &PostProcessing::multisampling;
	pipelineInfo.pColorBlendState = &PostProcessing::colorBlending;
	pipelineInfo.pDynamicState = &PostProcessing::dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = HDRPass::renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;

	vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
}

void Skybox::draw(VkCommandBuffer commandBuffer)
{
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
	vkCmdDraw(commandBuffer, 36, 1, 0, 0);
}

void Skybox::destroy()
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
