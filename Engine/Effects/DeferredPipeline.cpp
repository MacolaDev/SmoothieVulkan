#include "DeferredPipeline.h"
#include <iostream>
#include <unordered_map>

#include "Core/SmoothieCore.h"
#include "Core/Pipeline.h"
#include "Core/Descriptor.h"


using namespace Smoothie::DeferredRendering;

static std::vector<Smoothie::ShaderObjectReflection> get_all_reflections(const Smoothie::ShaderFile& file)
{
	std::vector<Smoothie::ShaderObjectReflection> result;
	for (const auto& [key, value]: file.shaders)
	{
		if (key.name != "STANDARD") continue;
		if (auto search = file.shaders.find(key); search != file.shaders.end())
		{
			const auto& reflection = file.shaders.at(key).reflections;
			result.insert(result.end(), reflection.begin(), reflection.end());
		}
	}

	return result;
}

int Smoothie::DeferredRendering::StaticPipeline::create(const ShaderFile& shaderFile)
{
	DeferredRendering::Drawing* __drawing_class = dynamic_cast<DeferredRendering::Drawing*>(SmoothieCore::getDrawerClass());
	if (__drawing_class == nullptr)
	{
		std::cout << "Failed to get drawing class!" << std::endl;
		return 1;
	}

	const std::vector< ShaderFile::SearchKey> __search_keys =
	{
		{ "STANDARD", VK_SHADER_STAGE_FRAGMENT_BIT }, { "STANDARD", VK_SHADER_STAGE_VERTEX_BIT },
		{ "STANDARD_SHADOW", VK_SHADER_STAGE_FRAGMENT_BIT }, { "STANDARD_SHADOW", VK_SHADER_STAGE_VERTEX_BIT }
	};

	const auto& shader = shaderFile;
	if (shader.shaders.find({ "STANDARD", VK_SHADER_STAGE_FRAGMENT_BIT }) == shader.shaders.end())
	{
		std::cout << "Failed to get the fragment shader with name STANDARD" << std::endl;
		return 1;
	}

	const int nOfPipelineOutputs = shader.shaders.at({ "STANDARD", VK_SHADER_STAGE_FRAGMENT_BIT }).pipe_outputs.size();
	if ((nOfPipelineOutputs != 4) && (nOfPipelineOutputs != 1))
	{
		std::cout << "Fragment shader in STANDARD must either have a 4 outputs for gBuffer or 1 output for HDR. No other outputs are allowed!" << std::endl;
		return 1;
	}


	if (m_DescriptorData.create(shaderFile, 2, __search_keys) != 0)
	{
		std::cout << "Failed to get data for descriptor set: " << 2 << std::endl;
		return 1;
	}


	drawingClassDescriptorSetLayout = __drawing_class->getDescriptorSetLayout(0);
	if (drawingClassDescriptorSetLayout == nullptr)
	{
		std::cout << "Descriptor set layout for drawing class is invalid!" << std::endl;
		return 1;
	}
	

	VkRenderPass renderPass = (nOfPipelineOutputs == 1) ? __drawing_class->getRenderPassHDR() : __drawing_class->getRenderPassGBuffer();
	
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
	colorBlending.attachmentCount = (nOfPipelineOutputs == 4) ? 4 : 1;
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
	
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();
	
	const auto& __descriptor_bindings = m_DescriptorData.getDescriptorBindings();
	VkDescriptorSetLayoutCreateInfo __descriptorSetLayoutCreateInfo = {};
	__descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	__descriptorSetLayoutCreateInfo.pNext = nullptr;
	__descriptorSetLayoutCreateInfo.flags = 0;
	__descriptorSetLayoutCreateInfo.bindingCount = __descriptor_bindings.size();
	__descriptorSetLayoutCreateInfo.pBindings = __descriptor_bindings.data();
	if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &__descriptorSetLayoutCreateInfo, nullptr, &m_PipelineDescriptorSetLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor set layout!" << std::endl;
		return 1;
	}


	const VkDescriptorSetLayout descriptorSets[]
	{
		SmoothieCore::getCameraDescriptorSetLayout(0),
		drawingClassDescriptorSetLayout,
		m_PipelineDescriptorSetLayout
	};
	
	//Pipeline layout creation info
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 3;
	pipelineLayoutInfo.pSetLayouts = descriptorSets;
	if(vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS)
	{
		std::cout << "Failed to create pipeline layout!" << std::endl;
		return 1;
	}


	std::vector<VkPipelineShaderStageCreateInfo> raster_shader_stages;

	const std::string shader_identifier = "STANDARD";
	if (shader.get_pipeline_data_shaders(shader_identifier, VK_SHADER_STAGE_VERTEX_BIT, raster_shader_stages) != 0)
	{
		std::cout << "Shader file: " << shader.get_filepath() << " does not have a vertex shader with name" << shader_identifier << std::endl;
		return 1;
	}

	//Optional stages
	shader.get_pipeline_data_shaders(shader_identifier, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, raster_shader_stages);
	shader.get_pipeline_data_shaders(shader_identifier, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, raster_shader_stages);
	shader.get_pipeline_data_shaders(shader_identifier, VK_SHADER_STAGE_GEOMETRY_BIT, raster_shader_stages);
	if (shader.get_pipeline_data_shaders(shader_identifier, VK_SHADER_STAGE_FRAGMENT_BIT, raster_shader_stages) != 0)
	{
		std::cout << "Shader file: " << shader.get_filepath() << " does not have a fragment shader with name" << shader_identifier << std::endl;
		return 1;
	}


	std::vector<VkVertexInputBindingDescription> vertexShaderBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> vertexShaderAttributeDescriptions;
	
	if (shader.get_pipeline_data_vertex_stage_data("STANDARD", vertexShaderBindingDescriptions, vertexShaderAttributeDescriptions) != 0)
	{
		std::cout << "Failed to get the pipeline vertex data!" << std::endl;
		return 1;
	}

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
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipeline_layout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;
	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create pipeline!" << std::endl;
		return 1;
	}
	
	return 0;
}

void Smoothie::DeferredRendering::StaticPipeline::bindAndDraw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, unsigned int ImageIndex) const
{
	if ((pipeline != nullptr) && (pipeline_layout != nullptr)) 
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		for (size_t i = 0; i < m_Models.size(); i++)
		{
			if (m_Models[i] == nullptr) continue;
			m_Models[i]->draw(commandBuffer, pipeline_layout, descriptorSet, ImageIndex);
		}
	}
}

void Smoothie::DeferredRendering::StaticPipeline::bindAndDrawShadow(const SmoothieMath::Matrix4x4& lightProjectionViewMat, VkCommandBuffer commandBuffer, unsigned int ImageIndex) const
{
	if ((shadow_pipeline != nullptr) && (shadow_pipelineLayout != nullptr))
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadow_pipeline);
		for (size_t i = 0; i < m_Models.size(); i++)
		{
			if (m_Models[i] == nullptr) continue;
			m_Models[i]->drawShadow(lightProjectionViewMat, shadow_pipelineLayout, commandBuffer, ImageIndex);
		}

	}
}

static std::mutex _add_to_rendering_mutex_GBuffer_pipeline;
void Smoothie::DeferredRendering::StaticPipeline::add_to_rendering(std::shared_ptr<DeferredModel> model) const
{
	std::lock_guard<std::mutex> lock(_add_to_rendering_mutex_GBuffer_pipeline);
	increaseUseCount();
	m_Models.push_back(model);
}

void Smoothie::DeferredRendering::StaticPipeline::remove_from_rendering(std::shared_ptr<DeferredModel> model) const
{
	//decreaseUseCount();
}

void Smoothie::DeferredRendering::StaticPipeline::destroy()
{
	for (size_t i = 0; i < m_Models.size(); i++)
	{
		DeferredModel* _model = m_Models[i].get();
		if (_model == nullptr) continue;

		_model->destroy();
		decreaseUseCount();
	}
	m_Models.clear();
	
	if (getUseCount() != 0) return;

	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), m_PipelineDescriptorSetLayout, nullptr);
	m_PipelineDescriptorSetLayout = nullptr;

	vkDestroyPipelineLayout(SmoothieCore::getDevice(), pipeline_layout, nullptr);
	pipeline_layout = nullptr;

	vkDestroyPipeline(SmoothieCore::getDevice(), pipeline, nullptr);
	pipeline = nullptr;

	if (shadow_pipelineLayout != nullptr) vkDestroyPipelineLayout(SmoothieCore::getDevice(), shadow_pipelineLayout, nullptr), shadow_pipelineLayout = nullptr;
	if (shadow_pipeline != nullptr) vkDestroyPipeline(SmoothieCore::getDevice(), shadow_pipeline, nullptr), shadow_pipeline = nullptr;

}
