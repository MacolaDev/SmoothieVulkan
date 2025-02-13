#include "PostProcessing.h"
#include "ResourceManager/ShaderFile.h"
#include <iostream>
#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"


VkPipelineViewportStateCreateInfo PostProcessing::viewportState{};
VkPipelineRasterizationStateCreateInfo PostProcessing::rasterizer{};
VkPipelineDepthStencilStateCreateInfo PostProcessing::pipelineDepthStencil{};
VkPipelineMultisampleStateCreateInfo PostProcessing::multisampling{};
VkPipelineColorBlendAttachmentState PostProcessing::colorBlendAttachment{};
VkPipelineColorBlendStateCreateInfo PostProcessing::colorBlending{};

VkPipelineVertexInputStateCreateInfo PostProcessing::standardVertexState{};
VkPipelineInputAssemblyStateCreateInfo PostProcessing::standardAssemblyState{};
VkPipelineDynamicStateCreateInfo PostProcessing::dynamicState{};
const VkDynamicState PostProcessing::dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

static const char main_function_name[] = "main";

void PostProcessing::create()
{
	//Viewport
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	//Rasterizer
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthClampEnable = false;
	rasterizer.rasterizerDiscardEnable = false;
	rasterizer.lineWidth = 1.0f;
	rasterizer.depthBiasEnable = false;

	//Depth 
	pipelineDepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencil.depthTestEnable = false;
	pipelineDepthStencil.depthWriteEnable = false;
	pipelineDepthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	pipelineDepthStencil.depthBoundsTestEnable = false;
	pipelineDepthStencil.stencilTestEnable = false;

	//Multisampling
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//Color blending stuff
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = false;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = false;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;

	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;
	
	//Pipeline input state vertex
	standardVertexState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	standardVertexState.vertexBindingDescriptionCount = 0;
	standardVertexState.pVertexBindingDescriptions = nullptr;
	standardVertexState.vertexAttributeDescriptionCount = 0;
	standardVertexState.pVertexAttributeDescriptions = nullptr;

	standardAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	standardAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	standardAssemblyState.primitiveRestartEnable = VK_FALSE;

	//Dynamic state
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;
}

void PostProcessing::destroy()
{
}
