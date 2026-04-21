#include "Deferred_Pipeline.h"

#include <assert.h>
#include <iostream>
#include <unordered_map>

#include "Core/SmoothieCore.h"
#include "Core/Descriptor.h"

using namespace Smoothie::DeferredRendering;

int BasePipeline::update()
{
	return 0;
}

int Smoothie::DeferredRendering::StaticPipeline::create()
{

	auto* _drawing_class = dynamic_cast<DeferredRendering::Drawing*>(SmoothieCore::getDrawingClass());
	if (_drawing_class == nullptr)
	{
		std::cout << "Failed to get drawing class!" << std::endl;
		assert(_drawing_class != nullptr);
		return 1;
	}


	// if (m_ShaderFile.shaders.find({ "STANDARD", VK_SHADER_STAGE_FRAGMENT_BIT }) == m_ShaderFile.shaders.end())
	// {
	// 	std::cout << "Failed to get the fragment shader with name STANDARD" << std::endl;
	// 	return 1;
	// }

	// const int nOfPipelineOutputs = m_ShaderFile.shaders.at({ "STANDARD", VK_SHADER_STAGE_FRAGMENT_BIT }).pipe_outputs.size();
	// if ((nOfPipelineOutputs != 4) && (nOfPipelineOutputs != 1))
	// {
	// 	std::cout << "Fragment shader in STANDARD must either have a 4 outputs for gBuffer or 1 output for HDR. No other outputs are allowed!" << std::endl;
	// 	return 1;
	// }
	//m_tmpRenderPass = nOfPipelineOutputs;

	// if (m_DescriptorData.create(m_ShaderFile, 2, __search_keys) != 0)
	// {
	// 	std::cout << "Failed to get data for descriptor set: " << 2 << std::endl;
	// 	return 1;
	// }


	VkDescriptorSetLayout _drawingClassDescriptorSetLayout = _drawing_class->getDescriptorSetLayout(0);
	if (_drawingClassDescriptorSetLayout == nullptr)
	{
		std::cout << "Descriptor set layout for drawing class is invalid!" << std::endl;
		return 1;
	}
	

	//VkRenderPass renderPass = (nOfPipelineOutputs == 1) ? _drawing_class->getRenderPassHDR() : _drawing_class->getRenderPassGBuffer();
	
	//Viewport
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	
	//Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
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
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = false;
	colorBlending.logicOp = VK_LOGIC_OP_CLEAR;
	VkPipelineColorBlendAttachmentState attachments[] = { colorBlendAttachment, colorBlendAttachment, colorBlendAttachment , colorBlendAttachment };
	colorBlending.pAttachments = attachments;
	//colorBlending.attachmentCount = (nOfPipelineOutputs == 4) ? 4 : 1;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;
	
	//Dynamic states
	const std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_CULL_MODE
	};
	
	VkPipelineDynamicStateCreateInfo _dynamicState{};
	_dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	_dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	_dynamicState.pDynamicStates = dynamicStates.data();
	
	//const auto& __descriptor_bindings = m_DescriptorData.getDescriptorBindings();
	VkDescriptorSetLayoutCreateInfo _descriptorSetLayoutCreateInfo = {};
	_descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	_descriptorSetLayoutCreateInfo.pNext = nullptr;
	_descriptorSetLayoutCreateInfo.flags = 0;
	// __descriptorSetLayoutCreateInfo.bindingCount = __descriptor_bindings.size();
	// __descriptorSetLayoutCreateInfo.pBindings = __descriptor_bindings.data();
	if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &_descriptorSetLayoutCreateInfo, nullptr, &m_PipelineDescriptorSetLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor set layout!" << std::endl;
		return 1;
	}


	const VkDescriptorSetLayout descriptorSets[]
	{
		SmoothieCore::getCameraDescriptorSetLayout(0),
		_drawingClassDescriptorSetLayout,
		m_PipelineDescriptorSetLayout
	};
	
	//Pipeline layout creation info
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 3;
	pipelineLayoutInfo.pSetLayouts = descriptorSets;
	if(vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create pipeline layout!" << std::endl;
		return 1;
	}


	std::vector<VkPipelineShaderStageCreateInfo> raster_shader_stages;

	const std::string shader_identifier = "STANDARD";
	// if (m_ShaderFile.get_pipeline_data_shaders(shader_identifier, VK_SHADER_STAGE_VERTEX_BIT, raster_shader_stages) != 0)
	// {
	// 	std::cout << "Shader file: " << m_ShaderFile.get_Filepath() << " does not have a vertex shader with name" << shader_identifier << std::endl;
	// 	return 1;
	// }
	//
	// //Optional stages
	// m_ShaderFile.get_pipeline_data_shaders(shader_identifier, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, raster_shader_stages);
	// m_ShaderFile.get_pipeline_data_shaders(shader_identifier, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, raster_shader_stages);
	// m_ShaderFile.get_pipeline_data_shaders(shader_identifier, VK_SHADER_STAGE_GEOMETRY_BIT, raster_shader_stages);
	// if (m_ShaderFile.get_pipeline_data_shaders(shader_identifier, VK_SHADER_STAGE_FRAGMENT_BIT, raster_shader_stages) != 0)
	// {
	// 	std::cout << "Shader file: " << m_ShaderFile.get_Filepath() << " does not have a fragment shader with name" << shader_identifier << std::endl;
	// 	return 1;
	// }


	std::vector<VkVertexInputBindingDescription> vertexShaderBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> vertexShaderAttributeDescriptions;
	
	// if (m_ShaderFile.get_pipeline_data_vertex_stage_data("STANDARD", vertexShaderBindingDescriptions, vertexShaderAttributeDescriptions) != 0)
	// {
	// 	std::cout << "Failed to get the pipeline vertex data!" << std::endl;
	// 	return 1;
	// }

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputInfo{};
	pipelineVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexShaderBindingDescriptions.size());
	pipelineVertexInputInfo.pVertexBindingDescriptions = vertexShaderBindingDescriptions.data();
	pipelineVertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexShaderAttributeDescriptions.size());
	pipelineVertexInputInfo.pVertexAttributeDescriptions = vertexShaderAttributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo pipelineIndexInputInfo{};
	pipelineIndexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineIndexInputInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineIndexInputInfo.primitiveRestartEnable = VK_FALSE;

	//Pipeline creation
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = raster_shader_stages.size();
	pipelineInfo.pStages = raster_shader_stages.data();
	pipelineInfo.pVertexInputState = &pipelineVertexInputInfo;
	pipelineInfo.pInputAssemblyState = &pipelineIndexInputInfo;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pDepthStencilState = &pipelineDepthStencil;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &_dynamicState;
	pipelineInfo.layout = m_PipelineLayout;
	pipelineInfo.renderPass = nullptr;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;
	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create pipeline!" << std::endl;
		return 1;
	}
	
	return 0;
}

void Smoothie::DeferredRendering::StaticPipeline::bindAndDraw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, unsigned int ImageIndex) const
{
	if ((m_Pipeline != nullptr) && (m_PipelineLayout != nullptr))
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
		for (size_t i = 0; i < m_Models.size(); i++)
		{
			if (m_Models[i] == nullptr) continue;
			m_Models[i]->draw(commandBuffer, m_PipelineLayout, descriptorSet, ImageIndex);
		}
	}
}

void Smoothie::DeferredRendering::StaticPipeline::bindAndDrawShadow(const SmoothieMath::Matrix4x4& lightProjectionViewMat, VkCommandBuffer commandBuffer, unsigned int ImageIndex) const
{
	if ((m_PipelineShadow != nullptr) && (m_PipelineShadowLayout != nullptr))
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineShadow);
		for (size_t i = 0; i < m_Models.size(); i++)
		{
			if (m_Models[i] == nullptr) continue;
			m_Models[i]->drawShadow(lightProjectionViewMat, m_PipelineShadowLayout, commandBuffer, ImageIndex);
		}

	}
}

void StaticPipeline::record_commands(const RecordCommandsData &buffers) const
{
	const VkCommandBuffer _shadow = buffers.commandBuffer_Shadow;
	VkCommandBuffer _draw = nullptr;
	if (m_tmpRenderPass == 4)
	{
		_draw = buffers.commandBuffer_gBuffer;
	} else if (m_tmpRenderPass == 1)
	{
		_draw = buffers.commandBuffer_HDR;
	}
	else
	{
		return;
	}

	if ((m_Pipeline != nullptr) && (m_PipelineLayout != nullptr))
	{
		vkCmdBindPipeline(_draw, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
	}

	bool _drawShadow = (m_PipelineShadow != nullptr) && (m_PipelineShadowLayout != nullptr);
	if (_drawShadow)
	{
		vkCmdBindPipeline(_shadow, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineShadow);
	}

	for (size_t i = 0; i < m_Models.size(); i++)
	{
		if (m_Models[i] == nullptr) continue;

		m_Models[i]->draw(_draw, m_PipelineLayout, buffers.descriptorSet_DrawClass, buffers.imageIndex);
		if (_drawShadow)
		{
			//m_Models[i]->drawShadow(_shadow, m_PipelineLayout, buffers.descriptorSet_DrawClass, ImageIndex);
		}

	}

}

static std::mutex _add_to_rendering_mutex_GBuffer_pipeline;
int Smoothie::DeferredRendering::StaticPipeline::add_to_rendering(std::shared_ptr<Deferred_Model> model) const
{
	std::lock_guard<std::mutex> lock(_add_to_rendering_mutex_GBuffer_pipeline);
	m_UseCount++;
	m_Models.push_back(model);
	return 0;
}

void Smoothie::DeferredRendering::StaticPipeline::remove_from_rendering(std::shared_ptr<Deferred_Model> model) const
{
	//m_UseCount--;
}

void Smoothie::DeferredRendering::StaticPipeline::destroy()
{
	for (size_t i = 0; i < m_Models.size(); i++)
	{
		Deferred_Model* _model = m_Models[i].get();
		if (_model == nullptr) continue;

		_model->destroy();
		m_UseCount--;
	}
	m_Models.clear();
	
	if (m_UseCount != 0) return;

	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), m_PipelineDescriptorSetLayout, nullptr);
	m_PipelineDescriptorSetLayout = nullptr;

	vkDestroyPipelineLayout(SmoothieCore::getDevice(), m_PipelineLayout, nullptr);
	m_PipelineLayout = nullptr;

	vkDestroyPipeline(SmoothieCore::getDevice(), m_Pipeline, nullptr);
	m_Pipeline = nullptr;

	if (m_PipelineShadowLayout != nullptr) vkDestroyPipelineLayout(SmoothieCore::getDevice(), m_PipelineShadowLayout, nullptr), m_PipelineShadowLayout = nullptr;
	if (m_PipelineShadow != nullptr) vkDestroyPipeline(SmoothieCore::getDevice(), m_PipelineShadow, nullptr), m_PipelineShadow = nullptr;

}
