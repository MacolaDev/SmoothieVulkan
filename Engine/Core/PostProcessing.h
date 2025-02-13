#pragma once
#include <vulkan/vulkan.h>

//Contains most basic structures for every pipeline creation for post-processing effects
//to prevent copying code for no reason
struct PostProcessing
{
	static VkPipelineViewportStateCreateInfo viewportState;
	static VkPipelineRasterizationStateCreateInfo rasterizer;
	static VkPipelineDepthStencilStateCreateInfo pipelineDepthStencil;
	static VkPipelineMultisampleStateCreateInfo multisampling;
	static VkPipelineColorBlendAttachmentState colorBlendAttachment;
	static VkPipelineColorBlendStateCreateInfo colorBlending;
	static VkPipelineVertexInputStateCreateInfo standardVertexState;
	static VkPipelineInputAssemblyStateCreateInfo standardAssemblyState;
	static VkPipelineDynamicStateCreateInfo dynamicState;

	const static VkDynamicState dynamicStates[2];


	static void create();
	static void destroy();
};
