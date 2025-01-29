#include "Model.h"
#include "ResourceManager/ParseXML.h"
#include "UniformBuffers.h"
#define _SMOOTHIE_ENGINE
#include "Core/SmoothieCore.h"
#include "Core/DeferredPipeline.h"
#include "Core/SwapChain.h"

using namespace Smoothie;


Model::Model(const std::string& file, SmoothieMath::Matrix4x4 modelMatrix)
{
    modelFile = file;
    ParseXML xmlfile = ParseXML(modelFile);
    modelMesh = Mesh(xmlfile.getElement("geometryFile").textContent);
    modelShader = Shader(xmlfile.getElement("shader").textContent);
	auto normalMatrix = modelMatrix.normalMatrix();
	modelMatrices.push_back({ modelMatrix, Matrix4x4(normalMatrix) });
	properties = ShaderProperty(xmlfile.getElement("property"), modelMatrices);

    //Viewport
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
	
	//Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthClampEnable = false;
	rasterizer.rasterizerDiscardEnable = false;
	rasterizer.lineWidth = 1.0f;
	rasterizer.depthBiasEnable = false;

	//Depth 
	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencil{};
	pipelineDepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencil.depthTestEnable = true;
	pipelineDepthStencil.depthWriteEnable = true;
	pipelineDepthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	pipelineDepthStencil.depthBoundsTestEnable = false;
	pipelineDepthStencil.stencilTestEnable = false;

	//Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//Color blending stuff
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = false;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = false;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	VkPipelineColorBlendAttachmentState attachments[] = { colorBlendAttachment, colorBlendAttachment, colorBlendAttachment , colorBlendAttachment };
	colorBlending.attachmentCount = 4; //TODO: Fix this for non-gBuffer models
	colorBlending.pAttachments = attachments;
	
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;
	
	//Dynamic states
	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();
	
	//Pipeline layout creation info
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	
	//TODO: Update this to support additional buffer for 
	VkDescriptorSetLayout descriptorSets[] =
	{
		StandardUnifromBuffers::buffer1.getDescriptorSetLayout(),
		StandardUnifromBuffers::buffer2.getDescriptorSetLayout(),
		properties.getDescriptorSetLayout()
	};

	pipelineLayoutInfo.setLayoutCount = 3;
	pipelineLayoutInfo.pSetLayouts = descriptorSets;
	
	vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &modelPipelineLayout);

	//Pipeline creation
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	auto stages = modelShader.getPipelineShaderStages();
	pipelineInfo.stageCount = static_cast<unsigned int>(stages.size());
	pipelineInfo.pStages = stages.data();
	pipelineInfo.pVertexInputState = &modelMesh.getPipelineVertexInput();
	pipelineInfo.pInputAssemblyState = &modelMesh.getPipelineIndexInput();
	
	
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pDepthStencilState = &pipelineDepthStencil;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = modelPipelineLayout;
	pipelineInfo.renderPass = DeferredPipeline::get_gBufferRenderPass();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;

	vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &modelPipeline);
}

void Smoothie::Model::bindAndDraw(VkCommandBuffer commandBuffer) const
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);
	modelMesh.bind(commandBuffer);
	
	//Dynamic properties of a pipeline
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;

	//TODO: UPDATE THIS
	viewport.width = static_cast<float>(1280);
	viewport.height = static_cast<float>(720);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = SwapChain::getSwapChainExtent();
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	VkDescriptorSet descriptorSets[] = 
	{
		StandardUnifromBuffers::buffer1.getDescriptorSet(),
		StandardUnifromBuffers::buffer2.getDescriptorSet(),
		properties.getDescriptorSet()
	};

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
		 
		modelPipelineLayout, 0, 
		3, descriptorSets, 
		0, nullptr);
	vkCmdDrawIndexed(commandBuffer, modelMesh.getNumberOfIndices(), 1, 0, 0, 0);
}


void Smoothie::Model::destroy()
{
	vkDestroyPipeline(SmoothieCore::getDevice(), modelPipeline, nullptr);
	modelPipeline = nullptr;
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), modelPipelineLayout, nullptr);
	modelPipelineLayout = nullptr;
	
	properties.destroy();
	modelShader.destroy();
	modelMesh.destroy();
}

std::string Smoothie::Model::getModelFile() const
{
    return modelFile;
}

std::string Smoothie::Model::getMeshFile() const
{
    return meshFile;
}
