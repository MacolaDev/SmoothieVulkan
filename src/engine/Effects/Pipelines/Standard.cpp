#include "Standard.h"

#include "Core/SmoothieCore.h"
#include "../Deferred_Core.h"


using namespace Smoothie::DeferredRendering;


static constexpr VkPipelineViewportStateCreateInfo s_ViewportState =
    {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .pNext = nullptr,
    .viewportCount = 1,
    .scissorCount = 1
};

static constexpr VkPipelineInputAssemblyStateCreateInfo s_PipelineIndexInputInfo =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, .pNext = nullptr,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE
};

static constexpr VkPipelineRasterizationStateCreateInfo s_Rasterizer =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, .pNext = nullptr,
    .depthClampEnable = false,
    .rasterizerDiscardEnable = false,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_FRONT_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = false,
    .lineWidth = 1.0f,
};

static constexpr VkPipelineDepthStencilStateCreateInfo s_PipelineDepthStencil =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, .pNext = nullptr,
    .depthTestEnable = true,
    .depthWriteEnable = true,
    .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
    .depthBoundsTestEnable = false,
    .stencilTestEnable = false,
};

static constexpr VkPipelineMultisampleStateCreateInfo s_Multisampling =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, .pNext = nullptr,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .sampleShadingEnable = VK_FALSE,
};

static constexpr VkPipelineColorBlendAttachmentState s_ColorBlendAttachment =
{
    .blendEnable = false,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp = VK_BLEND_OP_ADD,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
};

static constexpr VkDynamicState s_DynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_CULL_MODE};

static constexpr VkPipelineDynamicStateCreateInfo s_DynamicState =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, .pNext = nullptr,
    .dynamicStateCount = 3,
    .pDynamicStates = s_DynamicStates
};

int Smoothie::DeferredRendering::Pipeline_Standard::create(const std::string& shader)
{

	const auto& _drawing_class = std::dynamic_pointer_cast<DeferredRendering::Drawing>(SmoothieCore::getDrawingClassPtr());
	if (_drawing_class == nullptr)
	{
		//std::cout << "Failed to get drawing class!" << std::endl;
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


	VkDescriptorSetLayout _drawingClassDescriptorSetLayout = _drawing_class->getGlobalDescriptorSetLayout();
	if (_drawingClassDescriptorSetLayout == nullptr)
	{
		//std::cout << "Descriptor set layout for drawing class is invalid!" << std::endl;
		return 1;
	}
	

	
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = false;
	colorBlending.logicOp = VK_LOGIC_OP_CLEAR;
	VkPipelineColorBlendAttachmentState attachments[] = { s_ColorBlendAttachment, s_ColorBlendAttachment, s_ColorBlendAttachment , s_ColorBlendAttachment };
	colorBlending.pAttachments = attachments;
	//colorBlending.attachmentCount = (nOfPipelineOutputs == 4) ? 4 : 1;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	
	//const auto& __descriptor_bindings = m_DescriptorData.getDescriptorBindings();
	VkDescriptorSetLayoutCreateInfo _descriptorSetLayoutCreateInfo = {};
	_descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	_descriptorSetLayoutCreateInfo.pNext = nullptr;
	_descriptorSetLayoutCreateInfo.flags = 0;
	// __descriptorSetLayoutCreateInfo.bindingCount = __descriptor_bindings.size();
	// __descriptorSetLayoutCreateInfo.pBindings = __descriptor_bindings.data();
	if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &_descriptorSetLayoutCreateInfo, nullptr, &m_PipelineDescriptorSetLayout) != VK_SUCCESS)
	{
		//std::cout << "Failed to create descriptor set layout!" << std::endl;
		return 1;
	}


	const VkDescriptorSetLayout descriptorSets[]
	{
		//SmoothieCore::getCameraDescriptorSetLayout(0),
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
		//std::cout << "Failed to create pipeline layout!" << std::endl;
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


	//Pipeline creation
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = raster_shader_stages.size();
	pipelineInfo.pStages = raster_shader_stages.data();
	pipelineInfo.pVertexInputState = &pipelineVertexInputInfo;
	pipelineInfo.pInputAssemblyState = &s_PipelineIndexInputInfo;
	pipelineInfo.pViewportState = &s_ViewportState;
	pipelineInfo.pRasterizationState = &s_Rasterizer;
	pipelineInfo.pDepthStencilState = &s_PipelineDepthStencil;
	pipelineInfo.pMultisampleState = &s_Multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &s_DynamicState;
	pipelineInfo.layout = m_PipelineLayout;
	pipelineInfo.renderPass = nullptr;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;
	// if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
	// {
	// 	std::cout << "Failed to create pipeline!" << std::endl;
	// 	return 1;
	// }
	
	return 0;
}

void Pipeline_Standard::record_data(const Pipeline_Data &data) const
{
    VkPipeline _pipeline = nullptr;
    VkCommandBuffer _commandBuffer = nullptr;
    if (m_Pipeline_HDR != nullptr)
    {
        _pipeline = m_Pipeline_HDR;
        _commandBuffer = data.buffers.GP_HDR;
    }
    else
    {
        _pipeline = m_Pipeline_gBuffer;
        _commandBuffer = data.buffers.GP_gBuffer;
    }

	if ((_pipeline != nullptr) && (m_PipelineLayout != nullptr))
	{
		vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
	    for (size_t i = 0; i < m_Models.size(); i++)
	    {
	        if (m_Models[i] == nullptr) continue;
	        m_Models[i]->draw(_commandBuffer, m_PipelineLayout, data.descriptorSet, data.imageIndex);
	    }

	}

}

static std::mutex _add_to_rendering_mutex_GBuffer_pipeline;
int Smoothie::DeferredRendering::Pipeline_Standard::add_to_rendering(const std::shared_ptr<Model_Standard>& model)
{
	// std::lock_guard<std::mutex> lock(_add_to_rendering_mutex_GBuffer_pipeline);
	// m_Models.push_back(model);
	return 0;
}

void Smoothie::DeferredRendering::Pipeline_Standard::remove_from_rendering(const std::shared_ptr<Model_Standard>& model)
{
	//m_UseCount--;
}

void Smoothie::DeferredRendering::Pipeline_Standard::destroy()
{
	for (size_t i = 0; i < m_Models.size(); i++)
	{
		auto* _model = m_Models[i].get();
		if (_model == nullptr) continue;

		_model->destroy();
	}
	m_Models.clear();

	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), m_PipelineDescriptorSetLayout, nullptr);
	m_PipelineDescriptorSetLayout = nullptr;

	vkDestroyPipelineLayout(SmoothieCore::getDevice(), m_PipelineLayout, nullptr);
	m_PipelineLayout = nullptr;


    if (m_Pipeline_gBuffer != nullptr) vkDestroyPipeline(SmoothieCore::getDevice(), m_Pipeline_gBuffer, nullptr), m_Pipeline_gBuffer = nullptr;
    if (m_Pipeline_HDR != nullptr) vkDestroyPipeline(SmoothieCore::getDevice(), m_Pipeline_HDR, nullptr), m_Pipeline_HDR = nullptr;

	if (m_PipelineLayoutShadow != nullptr) vkDestroyPipelineLayout(SmoothieCore::getDevice(), m_PipelineLayoutShadow, nullptr), m_PipelineLayoutShadow = nullptr;
	if (m_PipelineShadow != nullptr) vkDestroyPipeline(SmoothieCore::getDevice(), m_PipelineShadow, nullptr), m_PipelineShadow = nullptr;

}
