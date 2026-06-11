#include "Deferred_Core.h"

#include "Core/SmoothieCore.h"
#include "Core/Constants.h"

using namespace Smoothie;


int DeferredRendering::Drawing::create_samplers()
{
	VkSamplerCreateInfo _create_info_Texture2DSampler{};
	_create_info_Texture2DSampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	_create_info_Texture2DSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	_create_info_Texture2DSampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	_create_info_Texture2DSampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	_create_info_Texture2DSampler.minFilter = VK_FILTER_NEAREST;
	_create_info_Texture2DSampler.magFilter = VK_FILTER_LINEAR;
	_create_info_Texture2DSampler.minLod = 0.0f;
	_create_info_Texture2DSampler.maxLod = SMOOTHIE_IMAGE_MAX_LOD_MODEL_IMAGE;
	_create_info_Texture2DSampler.unnormalizedCoordinates = false;
	_create_info_Texture2DSampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	_create_info_Texture2DSampler.unnormalizedCoordinates = false;
	if (vkCreateSampler(SmoothieCore::getDevice(), &_create_info_Texture2DSampler, nullptr, &m_Texture2DSampler) != VK_SUCCESS)
	{
	    std::cout << "Failed to create m_Texture2DSampler!" << std::endl;
	    return 1;
	}

	VkSamplerCreateInfo _create_info_ClampToEdgeLINEAR{};
	_create_info_ClampToEdgeLINEAR.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	_create_info_ClampToEdgeLINEAR.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	_create_info_ClampToEdgeLINEAR.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	_create_info_ClampToEdgeLINEAR.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	_create_info_ClampToEdgeLINEAR.minFilter = VK_FILTER_LINEAR;
	_create_info_ClampToEdgeLINEAR.magFilter = VK_FILTER_LINEAR;
	_create_info_ClampToEdgeLINEAR.minLod = 0.0f;
	_create_info_ClampToEdgeLINEAR.maxLod = SMOOTHIE_IMAGE_MAX_LOD_MODEL_IMAGE;
	_create_info_ClampToEdgeLINEAR.unnormalizedCoordinates = false;
	_create_info_ClampToEdgeLINEAR.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    if (vkCreateSampler(SmoothieCore::getDevice(), &_create_info_ClampToEdgeLINEAR, nullptr, &m_ClampToEdgeLINEAR) != VK_SUCCESS)
    {
        std::cout << "Failed to create m_ClampToEdgeLINEAR!" << std::endl;
        return 1;
    }

	VkSamplerCreateInfo _create_info_ClampToEdgeNEAREST = _create_info_ClampToEdgeLINEAR;
	_create_info_ClampToEdgeNEAREST.minFilter = VK_FILTER_NEAREST;
	_create_info_ClampToEdgeNEAREST.magFilter = VK_FILTER_NEAREST;
	_create_info_ClampToEdgeNEAREST.maxLod = 0.0f;
    if (vkCreateSampler(SmoothieCore::getDevice(), &_create_info_ClampToEdgeNEAREST, nullptr, &m_ClampToEdgeNEAREST) != VK_SUCCESS)
    {
        std::cout << "Failed to create m_ClampToEdgeNEAREST!" << std::endl;
        return 1;
    }

	VkSamplerCreateInfo _create_info__RepeatNEAREST = _create_info_Texture2DSampler;
	_create_info__RepeatNEAREST.maxLod = 0.0f;
    if (vkCreateSampler(SmoothieCore::getDevice(), &_create_info__RepeatNEAREST, nullptr, &m_RepeatNEAREST) != VK_SUCCESS)
    {
        std::cout << "Failed to create m_ClampToEdgeNEAREST!" << std::endl;
        return 1;
    }

	return 0;
}

void DeferredRendering::Drawing::destroy_samplers()
{
    if (m_RepeatNEAREST != nullptr){vkDestroySampler(SmoothieCore::getDevice(), m_RepeatNEAREST, nullptr); m_RepeatNEAREST = nullptr;}
    if (m_ClampToEdgeNEAREST != nullptr){vkDestroySampler(SmoothieCore::getDevice(), m_ClampToEdgeNEAREST, nullptr); m_ClampToEdgeNEAREST = nullptr;}
    if (m_ClampToEdgeLINEAR != nullptr){vkDestroySampler(SmoothieCore::getDevice(), m_ClampToEdgeLINEAR, nullptr); m_ClampToEdgeLINEAR = nullptr;}
    if (m_Texture2DSampler != nullptr){vkDestroySampler(SmoothieCore::getDevice(), m_Texture2DSampler, nullptr); m_Texture2DSampler = nullptr;}
}

int DeferredRendering::Drawing::create_camera()
{
    if (m_CameraBuffer.create(m_Camera.getBufferSize()) != 0)
    {
        std::cout << "Failed to create CameraBuffer!" << std::endl;
        return 1;
    }

    if (m_CameraBuffer.map() != 0)
    {
        std::cout << "Failed to map Camera Buffer!" << std::endl;
        return 1;
    }

    return 0;
}

void DeferredRendering::Drawing::destroy_camera()
{
    m_CameraBuffer.unmap();
    m_CameraBuffer.destroy();
}

static std::vector<char> g_CameraData;
void DeferredRendering::Drawing::update_camera(const Camera &camera)
{
    camera.getBufferData(g_CameraData);
    m_CameraBuffer.copy_data(g_CameraData);
}

int DeferredRendering::Drawing::create_present_pipeline()
{
    const VkDescriptorSetLayout _descriptorSetLayouts[] =
	{
		m_Global_DescriptorSetLayout,
		m_PresentPipeline_DescriptorSetLayout
	};

	VkPipelineLayoutCreateInfo _pipeline_layout_createInfo{};
	_pipeline_layout_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	_pipeline_layout_createInfo.setLayoutCount = std::size(_descriptorSetLayouts);
	_pipeline_layout_createInfo.pSetLayouts = _descriptorSetLayouts;
	if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &_pipeline_layout_createInfo, nullptr, &m_PresentPipeline_Layout) != 0)
	{
		std::cout << "Failed to create pipeline layout!" << std::endl;
		return 1;
	}

	VkPipelineShaderStageCreateInfo _shader_stage_vertex{};
	_shader_stage_vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	_shader_stage_vertex.pName = m_PresentPipeline_VertexShaderEntryName.c_str();
	_shader_stage_vertex.stage = VK_SHADER_STAGE_VERTEX_BIT;
	_shader_stage_vertex.module = m_PresentPipeline_ShaderFile.get_ShaderModule();
    assert(_shader_stage_vertex.module != nullptr);

	VkPipelineShaderStageCreateInfo _shader_stage_fragment{};
	_shader_stage_fragment.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	_shader_stage_fragment.pName = m_PresentPipeline_FragmentShaderEntryName.c_str();
	_shader_stage_fragment.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	_shader_stage_fragment.module = m_PresentPipeline_ShaderFile.get_ShaderModule();
    assert(_shader_stage_fragment.module != nullptr);

	const VkPipelineShaderStageCreateInfo _shader_stages[] = { _shader_stage_vertex, _shader_stage_fragment };

    VkGraphicsPipelineCreateInfo pipeline_createInfo{};
    lazy_populate_graphics_pipeline(pipeline_createInfo);
	pipeline_createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_createInfo.pNext = nullptr;
    pipeline_createInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
	pipeline_createInfo.stageCount = std::size(_shader_stages);
	pipeline_createInfo.pStages = _shader_stages;
    pipeline_createInfo.pTessellationState = nullptr;
    pipeline_createInfo.layout = m_PresentPipeline_Layout;
    pipeline_createInfo.renderPass = SmoothieCore::getDefaultRenderPass();
    pipeline_createInfo.subpass = 0;
    pipeline_createInfo.basePipelineHandle = nullptr;
    pipeline_createInfo.basePipelineIndex = 0;

    assert(pipeline_createInfo.layout != nullptr);
    assert(pipeline_createInfo.renderPass != nullptr);


	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), nullptr, 1, &pipeline_createInfo, nullptr, &m_PresentPipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create global illumination pipeline!" << std::endl;
		return 1;
	}

	return 0;
}

void DeferredRendering::Drawing::destroy_present_pipeline()
{
    if (m_PresentPipeline != nullptr)
    {
        vkDestroyPipeline(SmoothieCore::getDevice(), m_PresentPipeline, nullptr); m_PresentPipeline = nullptr;
    }

    if (m_PresentPipeline_Layout != nullptr)
    {
        vkDestroyPipelineLayout(SmoothieCore::getDevice(), m_PresentPipeline_Layout, nullptr), m_PresentPipeline_Layout = nullptr;
    }
}

int DeferredRendering::Drawing::create_present_descriptor_set()
{
    std::vector<VkDescriptorSetLayoutBinding> _bindings;
    std::vector<VkDescriptorPoolSize> _poolSizes;
   if (m_PresentPipeline_ShaderFile.get_DescriptorData(m_Present_DescriptorSetName, _bindings, _poolSizes) != 0)
   {
        std::cout << "Failed to get global descriptor set data!" << std::endl;
       return 1;
   }

    VkDescriptorSetLayoutCreateInfo _layoutCreateInfo{};
    _layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    _layoutCreateInfo.pNext = nullptr;
    _layoutCreateInfo.flags = 0;
    _layoutCreateInfo.bindingCount = static_cast<unsigned int>(_bindings.size());
    _layoutCreateInfo.pBindings = _bindings.data();

    VkDescriptorSetLayoutSupport _support{};
    _support.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT;
    _support.pNext = nullptr;
    vkGetDescriptorSetLayoutSupport(SmoothieCore::getDevice(), &_layoutCreateInfo, &_support);
    if (_support.supported == VK_FALSE)
    {
        std::cout << "Descriptor set is unsupported!" << std::endl;
        return 1;
    }

    if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &_layoutCreateInfo, nullptr, &m_PresentPipeline_DescriptorSetLayout) != VK_SUCCESS)
    {
        std::cout << "Failed to create present descriptor set!" << std::endl;
        return 1;
    }

    VkDescriptorPoolCreateInfo _poolCreateInfo{};
    _poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    _poolCreateInfo.pNext = nullptr;
    _poolCreateInfo.poolSizeCount = static_cast<unsigned int>(_poolSizes.size());
    _poolCreateInfo.pPoolSizes = _poolSizes.data();
    _poolCreateInfo.maxSets = 1;
    if (vkCreateDescriptorPool(SmoothieCore::getDevice(), &_poolCreateInfo, nullptr, &m_PresentPipeline_DescriptorPool) != VK_SUCCESS)
    {
        std::cout << "Failed to create present descriptor pool!" << std::endl;
        return 1;
    }

    VkDescriptorSetAllocateInfo _allocInfo = {};
    _allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    _allocInfo.pNext = nullptr;
    _allocInfo.descriptorPool = m_PresentPipeline_DescriptorPool;
    _allocInfo.descriptorSetCount = 1;
    _allocInfo.pSetLayouts = &m_PresentPipeline_DescriptorSetLayout;
    if (vkAllocateDescriptorSets(SmoothieCore::getDevice(), &_allocInfo, &m_PresentPipeline_DescriptorSet) != VK_SUCCESS)
    {
        std::cout << "Failed to create present descriptor set!" << std::endl;
        return 1;
    }

    return 0;
}

void DeferredRendering::Drawing::destroy_present_descriptor_set()
{
    if (m_PresentPipeline_DescriptorPool != nullptr)
    {
        vkDestroyDescriptorPool(SmoothieCore::getDevice(), m_PresentPipeline_DescriptorPool, nullptr);
        m_PresentPipeline_DescriptorPool = nullptr;
        m_PresentPipeline_DescriptorSet = nullptr;
    }

    if (m_Global_DescriptorSetLayout != nullptr)
    {
        vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), m_PresentPipeline_DescriptorSetLayout, nullptr);
        m_PresentPipeline_DescriptorSetLayout = nullptr;
    }
}

void DeferredRendering::Drawing::lazy_populate_graphics_pipeline(VkGraphicsPipelineCreateInfo &PipelineCreateInfo) const
{
    static VkPipelineVertexInputStateCreateInfo VertexStateCreateInfo{};
    VertexStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexStateCreateInfo.pNext = nullptr;
    VertexStateCreateInfo.vertexBindingDescriptionCount = 0;
    VertexStateCreateInfo.pVertexBindingDescriptions = nullptr;
    VertexStateCreateInfo.vertexAttributeDescriptionCount = 0;
    VertexStateCreateInfo.pVertexAttributeDescriptions = nullptr;

    static VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo{};
    InputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyStateCreateInfo.pNext = nullptr;
    InputAssemblyStateCreateInfo.flags = 0;
    InputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    static VkPipelineTessellationStateCreateInfo TessellationStateCreateInfo{};
    TessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    TessellationStateCreateInfo.pNext = nullptr;
    TessellationStateCreateInfo.flags = 0;
    TessellationStateCreateInfo.patchControlPoints = 0;

    static VkPipelineViewportStateCreateInfo ViewportStateCreateInfo{};
    ViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportStateCreateInfo.pNext = nullptr;
    ViewportStateCreateInfo.flags = 0;
    ViewportStateCreateInfo.viewportCount = 1;
    ViewportStateCreateInfo.pViewports = nullptr;
    ViewportStateCreateInfo.scissorCount = 1;
    ViewportStateCreateInfo.pScissors = nullptr;

    static VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo{};
    RasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizationStateCreateInfo.pNext = nullptr;
    RasterizationStateCreateInfo.flags = 0;
    RasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    RasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    RasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    RasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
    RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    RasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    RasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
    RasterizationStateCreateInfo.depthBiasClamp = 0.0f;
    RasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
    RasterizationStateCreateInfo.lineWidth = 1.0f;

    static VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo{};
    MultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    MultisampleStateCreateInfo.pNext = nullptr;
    MultisampleStateCreateInfo.flags = 0;
    MultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    MultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    MultisampleStateCreateInfo.minSampleShading = 0.0f;
    MultisampleStateCreateInfo.pSampleMask = nullptr;
    MultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
    MultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

    static VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo{};
    DepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    DepthStencilStateCreateInfo.pNext = nullptr;
    DepthStencilStateCreateInfo.flags = 0;
    DepthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
    DepthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
    DepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    DepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    DepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    DepthStencilStateCreateInfo.front = {};
    DepthStencilStateCreateInfo.back = {};
    DepthStencilStateCreateInfo.minDepthBounds = 0.0f;
    DepthStencilStateCreateInfo.maxDepthBounds = 1.0f;

    static VkPipelineColorBlendAttachmentState ColorBlendAttachmentState{};
    ColorBlendAttachmentState.blendEnable = VK_FALSE;
    ColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    ColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
    ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    static VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo{};
    ColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlendStateCreateInfo.pNext = nullptr;
    ColorBlendStateCreateInfo.flags = 0;
    ColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    ColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_CLEAR;
    ColorBlendStateCreateInfo.attachmentCount = 1;
    ColorBlendStateCreateInfo.pAttachments = &ColorBlendAttachmentState;
    ColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
    ColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
    ColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
    ColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

    static constexpr VkDynamicState DynamicState[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    static VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo{};
    DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicStateCreateInfo.pNext = nullptr;
    DynamicStateCreateInfo.flags = 0;
    DynamicStateCreateInfo.dynamicStateCount = std::size(DynamicState);
    DynamicStateCreateInfo.pDynamicStates = DynamicState;

	PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineCreateInfo.pNext = nullptr;
    PipelineCreateInfo.flags = 0;
	PipelineCreateInfo.stageCount = 0;
	PipelineCreateInfo.pStages = nullptr;
    PipelineCreateInfo.pVertexInputState = &VertexStateCreateInfo;
    PipelineCreateInfo.pInputAssemblyState = &InputAssemblyStateCreateInfo;
    PipelineCreateInfo.pTessellationState = &TessellationStateCreateInfo;
    PipelineCreateInfo.pViewportState = &ViewportStateCreateInfo;
    PipelineCreateInfo.pRasterizationState = &RasterizationStateCreateInfo;
    PipelineCreateInfo.pMultisampleState = &MultisampleStateCreateInfo;
    PipelineCreateInfo.pDepthStencilState = &DepthStencilStateCreateInfo;
    PipelineCreateInfo.pColorBlendState = &ColorBlendStateCreateInfo;
    PipelineCreateInfo.pDynamicState = &DynamicStateCreateInfo;
    PipelineCreateInfo.layout = nullptr;
    PipelineCreateInfo.renderPass = nullptr;
    PipelineCreateInfo.subpass = 0;
    PipelineCreateInfo.basePipelineHandle = nullptr;
    PipelineCreateInfo.basePipelineIndex = 0;
}

int DeferredRendering::Drawing::create_lighting()
{
    if (m_Light_Global == nullptr)
    {
        std::cout << "m_Light_Global is a nullptr!" << std::endl;
        return 1;
    }

    if (m_Light_Global->create(m_Lighting_Global_Shader) != 0)
    {
        std::cout << "Failed to create global lighting data!" << std::endl;
        return 1;
    }
    return 0;
}

void DeferredRendering::Drawing::destroy_lighting()
{
    if (m_Light_Global != nullptr)
    {
        m_Light_Global->destroy();
    }
}

int DeferredRendering::Drawing::create_global_descriptor_sets()
{
    std::vector<VkDescriptorSetLayoutBinding> _bindings;
    std::vector<VkDescriptorPoolSize> _poolSizes;
   if (m_PresentPipeline_ShaderFile.get_DescriptorData(m_Global_DescriptorSetName, _bindings, _poolSizes) != 0)
   {
        std::cout << "Failed to create global descriptor set!" << std::endl;
       return 1;
   }

    VkDescriptorSetLayoutCreateInfo _layoutCreateInfo{};
    _layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    _layoutCreateInfo.pNext = nullptr;
    _layoutCreateInfo.bindingCount = static_cast<unsigned int>(_bindings.size());
    _layoutCreateInfo.pBindings = _bindings.data();

    VkDescriptorSetLayoutSupport _support{};
    _support.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT;
    _support.pNext = nullptr;
    vkGetDescriptorSetLayoutSupport(SmoothieCore::getDevice(), &_layoutCreateInfo, &_support);
    if (_support.supported == VK_FALSE)
    {
        std::cout << "Descriptor set is unsupported!" << std::endl;
        return 1;
    }

    if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &_layoutCreateInfo, nullptr, &m_Global_DescriptorSetLayout) != VK_SUCCESS)
    {
        std::cout << "Failed to create global descriptor set!" << std::endl;
        return 1;
    }

    VkDescriptorPoolCreateInfo _poolCreateInfo{};
    _poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    _poolCreateInfo.pNext = nullptr;
    _poolCreateInfo.poolSizeCount = static_cast<unsigned int>(_poolSizes.size());
    _poolCreateInfo.pPoolSizes = _poolSizes.data();
    _poolCreateInfo.maxSets = 1;
    if (vkCreateDescriptorPool(SmoothieCore::getDevice(), &_poolCreateInfo, nullptr, &m_Global_DescriptorPool) != VK_SUCCESS)
    {
        std::cout << "Failed to create global descriptor pool!" << std::endl;
        return 1;
    }

    VkDescriptorSetAllocateInfo _allocInfo = {};
    _allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    _allocInfo.pNext = nullptr;
    _allocInfo.descriptorPool = m_Global_DescriptorPool;
    _allocInfo.descriptorSetCount = 1;
    _allocInfo.pSetLayouts = &m_Global_DescriptorSetLayout;
    if (vkAllocateDescriptorSets(SmoothieCore::getDevice(), &_allocInfo, &m_Global_DescriptorSet) != VK_SUCCESS)
    {
        std::cout << "Failed to create global descriptor set!" << std::endl;
        return 1;
    }

    return 0;
}

void DeferredRendering::Drawing::destroy_global_descriptor_sets()
{

    if (m_Global_DescriptorPool != nullptr)
    {
        vkDestroyDescriptorPool(SmoothieCore::getDevice(), m_Global_DescriptorPool, nullptr);
        m_Global_DescriptorPool = nullptr;
        m_Global_DescriptorSet = nullptr;
    }

    if (m_Global_DescriptorSetLayout != nullptr)
    {
        vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), m_Global_DescriptorSetLayout, nullptr);
        m_Global_DescriptorSetLayout = nullptr;
    }

}

void DeferredRendering::Drawing::populate_global_descriptor_set()
{

    const std::string _attachments_path_base = m_Global_DescriptorSetName + "." + m_Global_DescriptorVariableName_Attachments + ".";
    std::unordered_map<std::string, VkImageView> _attachments_map;
    _attachments_map[_attachments_path_base+"gPosition"] = m_gPosition.getImageView();
    _attachments_map[_attachments_path_base+"gNormal"] = m_gNormal.getImageView();
    _attachments_map[_attachments_path_base+"gAlbedo"] = m_gAlbedo.getImageView();
    _attachments_map[_attachments_path_base+"gMRAO"] = m_gMRAO.getImageView();
    _attachments_map[_attachments_path_base+"HDR"] = m_HDR.getImageView();
    _attachments_map[_attachments_path_base+"depth"] = m_Depth.getImageView();

    const std::string _samplers_path_base = m_Global_DescriptorSetName + "." + m_Global_DescriptorVariableName_Samplers + ".";
    std::unordered_map<std::string, VkSampler> _samplers_map;
    _samplers_map[_samplers_path_base+"Texture2DSampler"] = m_Texture2DSampler;
    _samplers_map[_samplers_path_base+"ClampToEdgeLINEAR"] = m_ClampToEdgeLINEAR;
    _samplers_map[_samplers_path_base+"ClampToEdgeNEAREST"] = m_ClampToEdgeNEAREST;
    _samplers_map[_samplers_path_base+"RepeatNEAREST"] = m_RepeatNEAREST;

    const std::string _buffers_map_name = m_Global_DescriptorSetName + ".";
    std::unordered_map<std::string, VkBuffer> _buffers_map;
    _buffers_map[_buffers_map_name+m_Global_DescriptorVariableName_Camera] = m_CameraBuffer.getBuffer();


    for (const auto& [_path, _data] :m_Global_DescriptorWriteData)
    {
        VkDescriptorImageInfo _image_info{};
        _image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        _image_info.imageView = SmoothieCore::getDefault2DTexture().getImageView();
        _image_info.sampler = nullptr;

        VkDescriptorBufferInfo _buffer_info{};
        _buffer_info.buffer = SmoothieCore::getDefaultBuffer().getBuffer();
        _buffer_info.offset = 0;
        _buffer_info.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet _descriptor_write = {};
        _descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        _descriptor_write.pNext = nullptr;
        _descriptor_write.dstSet = m_Global_DescriptorSet;
        _descriptor_write.dstBinding = _data.binding;
        _descriptor_write.dstArrayElement = 0;
        _descriptor_write.descriptorCount = 1;
        _descriptor_write.descriptorType = _data.type;
        _descriptor_write.pBufferInfo = nullptr;
        _descriptor_write.pImageInfo = nullptr;
        _descriptor_write.pTexelBufferView = nullptr;

        switch (_data.type)
        {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
            {
                if (_samplers_map.find(_path) == _samplers_map.end())
                {
                    std::cout << "Sampler set not found: " << _path << std::endl;
                    continue;
                }

                _image_info.imageView = nullptr;
                _image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                _image_info.sampler = _samplers_map.at(_path);
                _descriptor_write.pImageInfo = &_image_info;

            }break;

            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            {
                if (_attachments_map.find(_path) == _attachments_map.end())
                {
                    std::cout << "Sampler set not found: " << _path << std::endl;
                    continue;
                }

                _image_info.imageView = _attachments_map.at(_path);
                _image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                _image_info.sampler = nullptr;

                _descriptor_write.pImageInfo = &_image_info;
            } break;

            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            {

                if (_buffers_map.find(_path) == _buffers_map.end())
                {
                    _descriptor_write.pBufferInfo = &_buffer_info;
                    std::cout << "Updating buffer with a default uniform buffer: " << _path << std::endl;
                    continue;
                }
                _buffer_info.buffer = _buffers_map.at(_path);
                _descriptor_write.pBufferInfo = &_buffer_info;

            }break;

            default:
            {
                std::cout << "Unexpected descriptor type for global descriptor set: " << _path << std::endl;
                continue;
            } break;
        }

        vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_descriptor_write, 0, nullptr);
    }



}

int DeferredRendering::Drawing::create_command_buffers()
{

	m_DrawingBuffers.resize(SMOOTHIE_MAX_FRAMES_IN_FLIGHT);
	for (auto& _data: m_DrawingBuffers)
	{
		VkCommandPoolCreateInfo _poolCreateInfo{};
		_poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		_poolCreateInfo.pNext = nullptr;
		_poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		_poolCreateInfo.queueFamilyIndex = SmoothieCore::getQueueFamilyGraphicsIndex();
		if (vkCreateCommandPool(SmoothieCore::getDevice(), &_poolCreateInfo, nullptr, &_data._command_pool) != VK_SUCCESS)
		{
			std::cout << "Failed to create drawing command pool!" << std::endl;
			return 1;
		}

		VkCommandBufferAllocateInfo _commandBufferAllocateInfo{};
		_commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		_commandBufferAllocateInfo.commandPool = _data._command_pool;
		_commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

		VkCommandBuffer _commandBuffers[] =
		{
		    _data.CP_Culling,
		    _data.CP_Compute,
		    _data.GP_gBuffer,
		    _data.GP_Shadow,
		    _data.CP_SS_Effects,
		    _data.GP_HDR,
		    _data.CP_Post_processing,
		    _data.GP_Present
		};
		_commandBufferAllocateInfo.commandBufferCount = sizeof(_commandBuffers)/sizeof(VkCommandBuffer);
		if (vkAllocateCommandBuffers(SmoothieCore::getDevice(), &_commandBufferAllocateInfo, _commandBuffers) != VK_SUCCESS)
		{
			std::cout << "Failed to allocate command buffers!" << std::endl;
			return 1;
		}
		_data.CP_Culling = _commandBuffers[0];
		_data.CP_Compute = _commandBuffers[1];
		_data.GP_gBuffer = _commandBuffers[2];
		_data.GP_Shadow = _commandBuffers[3];
	    _data.CP_SS_Effects = _commandBuffers[4];
	    _data.GP_HDR = _commandBuffers[5];
	    _data.CP_Post_processing = _commandBuffers[6];
	    _data.GP_Present = _commandBuffers[7];
	}

	return 0;
}

void DeferredRendering::Drawing::destroy_command_buffers()
{
	for (auto& _data: m_DrawingBuffers)
	{
		vkDestroyCommandPool(SmoothieCore::getDevice(), _data._command_pool, nullptr), _data._command_pool = nullptr;
	    _data.CP_Culling = nullptr;
	    _data.CP_Compute = nullptr;
	    _data.GP_gBuffer = nullptr;
	    _data.GP_Shadow = nullptr;
	    _data.CP_SS_Effects = nullptr;
	    _data.GP_HDR = nullptr;
	    _data.CP_Post_processing = nullptr;
	    _data.GP_Present = nullptr;
	}
}

void DeferredRendering::Drawing::begin_command_buffers(unsigned int frame) const
{
	m_CurrentDrawingBuffer = m_DrawingBuffers[frame];

	VkCommandBufferBeginInfo _beginInfo{};
	_beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	_beginInfo.pNext = nullptr;
	_beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkCommandBufferInheritanceRenderingInfo _rendering_info{};
    populateInheritanceRenderingInfo_gBuffer(_rendering_info);

	VkCommandBufferInheritanceInfo _inheritanceInfo{};
	_inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	_inheritanceInfo.pNext = &_rendering_info;
	_inheritanceInfo.subpass = 0;
	_inheritanceInfo.framebuffer = nullptr;
	_inheritanceInfo.renderPass = nullptr;

	_beginInfo.pInheritanceInfo = &_inheritanceInfo;
	vkBeginCommandBuffer(m_CurrentDrawingBuffer.GP_gBuffer, &_beginInfo);

    populateInheritanceRenderingInfo_HDR(_rendering_info);
	_beginInfo.pInheritanceInfo = &_inheritanceInfo;
	vkBeginCommandBuffer(m_CurrentDrawingBuffer.GP_HDR, &_beginInfo);

	//vkBeginCommandBuffer(m_commandBuffer_Shadow, &_beginInfo);

	_beginInfo.flags = 0;
	_beginInfo.pInheritanceInfo = &_inheritanceInfo;
	vkBeginCommandBuffer(m_CurrentDrawingBuffer.CP_Compute, &_beginInfo);
}

void DeferredRendering::Drawing::end_command_buffers() const
{
	vkEndCommandBuffer(m_CurrentDrawingBuffer.GP_gBuffer);
	vkEndCommandBuffer(m_CurrentDrawingBuffer.GP_HDR);
	//vkEndCommandBuffer(m_commandBuffer_Shadow);
	vkEndCommandBuffer(m_CurrentDrawingBuffer.CP_Compute);
}

int DeferredRendering::Drawing::create_gBuffer()
{
    const VkExtent3D _extent = { SmoothieCore::getScrWidth() , SmoothieCore::getScrHeight() , 1 };
    constexpr VkImageUsageFlags _usage_color = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    constexpr VkImageUsageFlags _usage_depth = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    int _err = 0;
    _err = m_gPosition.create(_extent, VK_FORMAT_R32G32B32A32_SFLOAT, _usage_color, VK_IMAGE_ASPECT_COLOR_BIT);
    if (_err != 0) return -1;

    _err = m_gNormal.create(_extent, VK_FORMAT_R16G16B16A16_SFLOAT, _usage_color, VK_IMAGE_ASPECT_COLOR_BIT);
    if (_err != 0) return -1;

    _err = m_gAlbedo.create(_extent, VK_FORMAT_R8G8B8A8_UNORM, _usage_color, VK_IMAGE_ASPECT_COLOR_BIT);
    if (_err != 0) return -1;

    _err = m_gMRAO.create(_extent, VK_FORMAT_R8G8B8A8_UNORM, _usage_color, VK_IMAGE_ASPECT_COLOR_BIT);
    if (_err != 0) return -1;

    _err = m_Depth.create(_extent, VK_FORMAT_D32_SFLOAT_S8_UINT, _usage_depth,  VK_IMAGE_ASPECT_DEPTH_BIT);
    if (_err != 0) return -1;

    return 0;
}

void DeferredRendering::Drawing::destroy_gBuffer()
{
    m_Depth.destroy();
    m_gMRAO.destroy();
    m_gAlbedo.destroy();
    m_gNormal.destroy();
    m_gPosition.destroy();
}

static constexpr VkFormat g_gBufferInheritanceFormats[] =
{
    VK_FORMAT_R32G32B32A32_SFLOAT,
    VK_FORMAT_R16G16B16A16_SFLOAT,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_R8G8B8A8_UNORM,
};
void DeferredRendering::Drawing::populateInheritanceRenderingInfo_gBuffer(
    VkCommandBufferInheritanceRenderingInfo &inheritance_info) const
{
    inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
    inheritance_info.pNext = nullptr;
    inheritance_info.flags = 0;
    inheritance_info.viewMask = 0;
    inheritance_info.colorAttachmentCount = std::size(g_gBufferInheritanceFormats);
    inheritance_info.pColorAttachmentFormats = g_gBufferInheritanceFormats;
    inheritance_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    inheritance_info.stencilAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    inheritance_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
}

static constexpr VkFormat g_HDRInheritanceFormats[] =
{
    VK_FORMAT_R16G16B16A16_SFLOAT
};
void DeferredRendering::Drawing::populateInheritanceRenderingInfo_HDR(
    VkCommandBufferInheritanceRenderingInfo &inheritance_info) const
{
    inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
    inheritance_info.pNext = nullptr;
    inheritance_info.flags = 0;
    inheritance_info.viewMask = 0;
    inheritance_info.colorAttachmentCount = std::size(g_HDRInheritanceFormats);
    inheritance_info.pColorAttachmentFormats = g_HDRInheritanceFormats;
    inheritance_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    inheritance_info.stencilAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    inheritance_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
}

void DeferredRendering::Drawing::begin_gBuffer(VkCommandBuffer commandBuffer) const
{
    VkImageSubresourceRange _color_subresource{};
    _color_subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _color_subresource.baseMipLevel = 0;
    _color_subresource.levelCount = 1;
    _color_subresource.baseArrayLayer = 0;
    _color_subresource.layerCount = 1;

    VkImageMemoryBarrier2 _barrier_color{};
    _barrier_color.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _barrier_color.pNext = nullptr;
    _barrier_color.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    _barrier_color.srcAccessMask = VK_ACCESS_2_NONE;
    _barrier_color.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    _barrier_color.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    _barrier_color.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _barrier_color.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _barrier_color.srcQueueFamilyIndex = 0;
    _barrier_color.dstQueueFamilyIndex = 0;
    _barrier_color.image = nullptr;
    _barrier_color.subresourceRange = _color_subresource;

    VkImageSubresourceRange _depth_subresource{};
    _depth_subresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    _depth_subresource.baseMipLevel = 0;
    _depth_subresource.levelCount = 1;
    _depth_subresource.baseArrayLayer = 0;
    _depth_subresource.layerCount = 1;

    VkImageMemoryBarrier2 _barrier_depth_stencil{};
    _barrier_depth_stencil.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _barrier_depth_stencil.pNext = nullptr;
    _barrier_depth_stencil.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    _barrier_depth_stencil.srcAccessMask = VK_ACCESS_2_NONE;
    _barrier_depth_stencil.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
    _barrier_depth_stencil.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    _barrier_depth_stencil.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _barrier_depth_stencil.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    _barrier_depth_stencil.srcQueueFamilyIndex = 0;
    _barrier_depth_stencil.dstQueueFamilyIndex = 0;
    _barrier_depth_stencil.image = nullptr;
    _barrier_depth_stencil.subresourceRange = _depth_subresource;

    VkImageMemoryBarrier2 _barrier_gPosition = _barrier_color;
    _barrier_gPosition.image = m_gPosition.getImage();
    VkImageMemoryBarrier2 _barrier_normal = _barrier_color;
    _barrier_normal.image = m_gNormal.getImage();
    VkImageMemoryBarrier2 _barrier_albedo = _barrier_color;
    _barrier_albedo.image = m_gAlbedo.getImage();
    VkImageMemoryBarrier2 _barrier_mrao = _barrier_color;
    _barrier_mrao.image = m_gMRAO.getImage();
    VkImageMemoryBarrier2 _barrier_depth = _barrier_depth_stencil;
    _barrier_depth.image = m_Depth.getImage();
    const VkImageMemoryBarrier2 _barriers[] = {_barrier_gPosition, _barrier_normal, _barrier_albedo, _barrier_mrao, _barrier_depth};

    VkDependencyInfo _dependencyInfo = {};
    _dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    _dependencyInfo.pNext = nullptr;
    _dependencyInfo.dependencyFlags = 0;
    _dependencyInfo.memoryBarrierCount = 0;
    _dependencyInfo.pMemoryBarriers = nullptr;
    _dependencyInfo.bufferMemoryBarrierCount = 0;
    _dependencyInfo.pBufferMemoryBarriers = nullptr;
    _dependencyInfo.imageMemoryBarrierCount = std::size(_barriers);
    _dependencyInfo.pImageMemoryBarriers = _barriers;
    vkCmdPipelineBarrier2(commandBuffer, &_dependencyInfo);

    VkRenderingAttachmentInfo _attachment_base{};
    _attachment_base.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    _attachment_base.pNext = nullptr;
    _attachment_base.imageView = nullptr;
    _attachment_base.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _attachment_base.resolveMode = VK_RESOLVE_MODE_NONE;
    _attachment_base.resolveImageView = nullptr;
    _attachment_base.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _attachment_base.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    _attachment_base.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    _attachment_base.clearValue.color.float32[0] = 0.0f;
    _attachment_base.clearValue.color.float32[1] = 0.0f;
    _attachment_base.clearValue.color.float32[2] = 0.0f;
    _attachment_base.clearValue.color.float32[3] = 1.0f;

    VkRenderingAttachmentInfo _g_position = _attachment_base;
    _g_position.imageView = m_gPosition.getImageView();
    VkRenderingAttachmentInfo _g_normal = _attachment_base;
    _g_normal.imageView = m_gNormal.getImageView();
    VkRenderingAttachmentInfo _g_albedo = _attachment_base;
    _g_albedo.imageView = m_gAlbedo.getImageView();
    VkRenderingAttachmentInfo _g_mrao = _attachment_base;
    _g_mrao.imageView = m_gMRAO.getImageView();
    const VkRenderingAttachmentInfo _color_attachments[] = {_g_position, _g_normal, _g_albedo, _g_mrao};

    VkRenderingAttachmentInfo _depth = _attachment_base;
    _depth.imageView = m_Depth.getImageView();
    _depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    _depth.clearValue.depthStencil.depth = 1.0f;
    _depth.clearValue.depthStencil.stencil = 0;

    VkRenderingInfo _renderingInfo{};
    _renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    _renderingInfo.pNext = nullptr;
    _renderingInfo.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
    _renderingInfo.renderArea.extent = {SmoothieCore::getScrWidth() , SmoothieCore::getScrHeight()};
    _renderingInfo.renderArea.offset = {0, 0};
    _renderingInfo.layerCount = 1;
    _renderingInfo.viewMask = 0;
    _renderingInfo.pColorAttachments = _color_attachments;
    _renderingInfo.colorAttachmentCount = std::size(_color_attachments);
    _renderingInfo.pDepthAttachment = &_depth;
    _renderingInfo.pStencilAttachment = &_depth;
    vkCmdBeginRendering(commandBuffer, &_renderingInfo);
}

void DeferredRendering::Drawing::end_gBuffer(VkCommandBuffer commandBuffer) const
{
    vkCmdEndRendering(commandBuffer);

    VkImageSubresourceRange _color_subresource{};
    _color_subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _color_subresource.baseMipLevel = 0;
    _color_subresource.levelCount = 1;
    _color_subresource.baseArrayLayer = 0;
    _color_subresource.layerCount = 1;

    VkImageMemoryBarrier2 _barrier_color{};
    _barrier_color.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _barrier_color.pNext = nullptr;
    _barrier_color.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    _barrier_color.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    _barrier_color.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _barrier_color.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
    _barrier_color.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _barrier_color.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    _barrier_color.srcQueueFamilyIndex = 0;
    _barrier_color.dstQueueFamilyIndex = 0;
    _barrier_color.image = nullptr;
    _barrier_color.subresourceRange = _color_subresource;

    VkImageSubresourceRange _depth_subresource{};
    _depth_subresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    _depth_subresource.baseMipLevel = 0;
    _depth_subresource.levelCount = 1;
    _depth_subresource.baseArrayLayer = 0;
    _depth_subresource.layerCount = 1;

    VkImageMemoryBarrier2 _barrier_depth_stencil{};
    _barrier_depth_stencil.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _barrier_depth_stencil.pNext = nullptr;
    _barrier_depth_stencil.srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT ;
    _barrier_depth_stencil.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    _barrier_depth_stencil.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _barrier_depth_stencil.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    _barrier_depth_stencil.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    _barrier_depth_stencil.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    _barrier_depth_stencil.srcQueueFamilyIndex = 0;
    _barrier_depth_stencil.dstQueueFamilyIndex = 0;
    _barrier_depth_stencil.image = nullptr;
    _barrier_depth_stencil.subresourceRange = _depth_subresource;

    VkImageMemoryBarrier2 _barrier_gPosition = _barrier_color;
    _barrier_gPosition.image = m_gPosition.getImage();
    VkImageMemoryBarrier2 _barrier_normal = _barrier_color;
    _barrier_normal.image = m_gNormal.getImage();
    VkImageMemoryBarrier2 _barrier_albedo = _barrier_color;
    _barrier_albedo.image = m_gAlbedo.getImage();
    VkImageMemoryBarrier2 _barrier_mrao = _barrier_color;
    _barrier_mrao.image = m_gMRAO.getImage();
    VkImageMemoryBarrier2 _barrier_depth = _barrier_depth_stencil;
    _barrier_depth.image = m_Depth.getImage();
    const VkImageMemoryBarrier2 _barriers[] = {_barrier_gPosition, _barrier_normal, _barrier_albedo, _barrier_mrao, _barrier_depth};

    VkDependencyInfo _dependencyInfo = {};
    _dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    _dependencyInfo.pNext = nullptr;
    _dependencyInfo.dependencyFlags = 0;
    _dependencyInfo.memoryBarrierCount = 0;
    _dependencyInfo.pMemoryBarriers = nullptr;
    _dependencyInfo.bufferMemoryBarrierCount = 0;
    _dependencyInfo.pBufferMemoryBarriers = nullptr;
    _dependencyInfo.imageMemoryBarrierCount = std::size(_barriers);
    _dependencyInfo.pImageMemoryBarriers = _barriers;
    vkCmdPipelineBarrier2(commandBuffer, &_dependencyInfo);

}

void DeferredRendering::Drawing::getRenderingInfo_gBuffer(VkPipelineRenderingCreateInfo &renderingInfo) const
{
    static constexpr VkFormat _color_attachments[] ={VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM,VK_FORMAT_R8G8B8A8_UNORM};

    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingInfo.pNext = nullptr;
    renderingInfo.viewMask = 0;
    renderingInfo.colorAttachmentCount = std::size(_color_attachments);
    renderingInfo.pColorAttachmentFormats = _color_attachments;
    renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
}

void DeferredRendering::Drawing::getRenderingInfo_HDR(VkPipelineRenderingCreateInfo &renderingInfo) const
{
    static constexpr VkFormat _color_attachments[] = {VK_FORMAT_R16G16B16A16_SFLOAT};

    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingInfo.pNext = nullptr;
    renderingInfo.viewMask = 0;
    renderingInfo.colorAttachmentCount = std::size(_color_attachments);
    renderingInfo.pColorAttachmentFormats = _color_attachments;
    renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
}

int DeferredRendering::Drawing::create_hdr()
{
    const VkExtent3D _extent = { SmoothieCore::getScrWidth() , SmoothieCore::getScrHeight() , 1 };
    int _err = 0;
    _err = m_HDR.create(_extent, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
    if (_err != 0) return -1;

    return 0;
}

void DeferredRendering::Drawing::destroy_hdr()
{
    m_HDR.destroy();
}

void DeferredRendering::Drawing::begin_hdr(VkCommandBuffer commandBuffer) const
{
   VkImageSubresourceRange _color_subresource{};
    _color_subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _color_subresource.baseMipLevel = 0;
    _color_subresource.levelCount = 1;
    _color_subresource.baseArrayLayer = 0;
    _color_subresource.layerCount = 1;

    VkImageMemoryBarrier2 _barrier_color{};
    _barrier_color.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _barrier_color.pNext = nullptr;
    _barrier_color.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    _barrier_color.srcAccessMask = VK_ACCESS_2_NONE;
    _barrier_color.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    _barrier_color.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    _barrier_color.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _barrier_color.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _barrier_color.srcQueueFamilyIndex = 0;
    _barrier_color.dstQueueFamilyIndex = 0;
    _barrier_color.image = m_HDR.getImage();
    _barrier_color.subresourceRange = _color_subresource;

    VkImageSubresourceRange _depth_subresource{};
    _depth_subresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    _depth_subresource.baseMipLevel = 0;
    _depth_subresource.levelCount = 1;
    _depth_subresource.baseArrayLayer = 0;
    _depth_subresource.layerCount = 1;

    VkImageMemoryBarrier2 _barrier_depth_stencil{};
    _barrier_depth_stencil.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _barrier_depth_stencil.pNext = nullptr;
    _barrier_depth_stencil.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _barrier_depth_stencil.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    _barrier_depth_stencil.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
    _barrier_depth_stencil.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    _barrier_depth_stencil.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    _barrier_depth_stencil.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    _barrier_depth_stencil.srcQueueFamilyIndex = 0;
    _barrier_depth_stencil.dstQueueFamilyIndex = 0;
    _barrier_depth_stencil.image = m_Depth.getImage();
    _barrier_depth_stencil.subresourceRange = _depth_subresource;

    const VkImageMemoryBarrier2 _barriers[] = {_barrier_color, _barrier_depth_stencil};

    VkDependencyInfo _dependencyInfo = {};
    _dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    _dependencyInfo.pNext = nullptr;
    _dependencyInfo.dependencyFlags = 0;
    _dependencyInfo.memoryBarrierCount = 0;
    _dependencyInfo.pMemoryBarriers = nullptr;
    _dependencyInfo.bufferMemoryBarrierCount = 0;
    _dependencyInfo.pBufferMemoryBarriers = nullptr;
    _dependencyInfo.imageMemoryBarrierCount = std::size(_barriers);
    _dependencyInfo.pImageMemoryBarriers = _barriers;
    vkCmdPipelineBarrier2(commandBuffer, &_dependencyInfo);


    VkRenderingAttachmentInfo _attachment_color{};
    _attachment_color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    _attachment_color.pNext = nullptr;
    _attachment_color.imageView = m_HDR.getImageView();
    _attachment_color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _attachment_color.resolveMode = VK_RESOLVE_MODE_NONE;
    _attachment_color.resolveImageView = nullptr;
    _attachment_color.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _attachment_color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    _attachment_color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    _attachment_color.clearValue.color.float32[0] = 0.0f;
    _attachment_color.clearValue.color.float32[1] = 0.0f;
    _attachment_color.clearValue.color.float32[2] = 0.0f;
    _attachment_color.clearValue.color.float32[3] = 1.0f;

    VkRenderingAttachmentInfo _depth = _attachment_color;
    _depth.imageView = m_Depth.getImageView();
    _depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    _depth.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    _depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    _depth.clearValue.depthStencil.depth = 1.0f;
    _depth.clearValue.depthStencil.stencil = 0;

    VkRenderingInfo _renderingInfo{};
    _renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    _renderingInfo.pNext = nullptr;
    _renderingInfo.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
    _renderingInfo.renderArea.extent = {SmoothieCore::getScrWidth() , SmoothieCore::getScrHeight()};
    _renderingInfo.renderArea.offset = {0, 0};
    _renderingInfo.layerCount = 1;
    _renderingInfo.viewMask = 0;
    _renderingInfo.pColorAttachments = &_attachment_color;
    _renderingInfo.colorAttachmentCount = 1;
    _renderingInfo.pDepthAttachment = &_depth;
    _renderingInfo.pStencilAttachment = &_depth;
    vkCmdBeginRendering(commandBuffer, &_renderingInfo);
}

void DeferredRendering::Drawing::end_hdr(VkCommandBuffer commandBuffer) const
{
    vkCmdEndRendering(commandBuffer);


    VkImageSubresourceRange _color_subresource{};
    _color_subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _color_subresource.baseMipLevel = 0;
    _color_subresource.levelCount = 1;
    _color_subresource.baseArrayLayer = 0;
    _color_subresource.layerCount = 1;

    VkImageMemoryBarrier2 _barrier_color{};
    _barrier_color.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _barrier_color.pNext = nullptr;
    _barrier_color.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    _barrier_color.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    _barrier_color.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _barrier_color.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
    _barrier_color.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _barrier_color.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    _barrier_color.srcQueueFamilyIndex = 0;
    _barrier_color.dstQueueFamilyIndex = 0;
    _barrier_color.image = m_HDR.getImage();
    _barrier_color.subresourceRange = _color_subresource;

    VkImageSubresourceRange _depth_subresource{};
    _depth_subresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    _depth_subresource.baseMipLevel = 0;
    _depth_subresource.levelCount = 1;
    _depth_subresource.baseArrayLayer = 0;
    _depth_subresource.layerCount = 1;

    VkImageMemoryBarrier2 _barrier_depth_stencil{};
    _barrier_depth_stencil.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _barrier_depth_stencil.pNext = nullptr;
    _barrier_depth_stencil.srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT ;
    _barrier_depth_stencil.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    _barrier_depth_stencil.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _barrier_depth_stencil.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    _barrier_depth_stencil.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    _barrier_depth_stencil.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    _barrier_depth_stencil.srcQueueFamilyIndex = 0;
    _barrier_depth_stencil.dstQueueFamilyIndex = 0;
    _barrier_depth_stencil.image = m_Depth.getImage();
    _barrier_depth_stencil.subresourceRange = _depth_subresource;

    const VkImageMemoryBarrier2 _barriers[] = {_barrier_color, _barrier_depth_stencil};

    VkDependencyInfo _dependencyInfo = {};
    _dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    _dependencyInfo.pNext = nullptr;
    _dependencyInfo.dependencyFlags = 0;
    _dependencyInfo.memoryBarrierCount = 0;
    _dependencyInfo.pMemoryBarriers = nullptr;
    _dependencyInfo.bufferMemoryBarrierCount = 0;
    _dependencyInfo.pBufferMemoryBarriers = nullptr;
    _dependencyInfo.imageMemoryBarrierCount = std::size(_barriers);
    _dependencyInfo.pImageMemoryBarriers = _barriers;
    vkCmdPipelineBarrier2(commandBuffer, &_dependencyInfo);

}

int DeferredRendering::Drawing::create_model_pipelines()
{
    assert(m_Pipeline_Sky != nullptr);
    if (m_Pipeline_Sky->create(m_Pipeline_Sky_Shader) != 0)
    {
        std::cout << "Failed to create sky pipeline!" << std::endl;
        return 1;
    }


    return 0;
}

void DeferredRendering::Drawing::destroy_model_pipelines()
{
    assert(m_Pipeline_Sky != nullptr);
    m_Pipeline_Sky->destroy();
}

int DeferredRendering::Drawing::create()
{
	if (create_command_buffers())
	{
		std::cout << "Failed to create draw command buffer data!" << std::endl;
	    destroy();
		return 1;
	}

    if (create_gBuffer() != 0)
    {
        std::cout << "Failed to create gBuffer data!" << std::endl;
        destroy();
        return 1;
    }

    if (create_hdr() != 0)
    {
        std::cout << "Failed to create HDR pass!" << std::endl;
        destroy();
        return 1;
    }

    if (create_samplers() != 0)
    {
        std::cout << "Failed to create samplers!" << std::endl;
        destroy();
        return 1;
    }

    if (create_camera() != 0)
    {
        std::cout << "Failed to create camera object!" << std::endl;
        destroy();
        return 1;
    }

    //TODO: Fix this garbage
    if (m_PresentPipeline_ShaderFile.create(m_PresentPipeline_Shader) != 0)
    {
        std::cout << "Failed to create present pipeline shader!" << std::endl;
        destroy();
        return 1;
    }

    std::unordered_map<std::string, WriteData_Buffer> _b;
    if (m_PresentPipeline_ShaderFile.get_DescriptorData(m_Global_DescriptorSetName, m_Global_DescriptorWriteData, _b) != 0)
    {
        std::cout << "Failed to get WriteData_Descriptor for a global descriptor set!" << std::endl;
        destroy();
        return 1;
    }

    if (create_global_descriptor_sets() != 0)
    {
        std::cout << "Failed to create global descriptor sets!" << std::endl;
        destroy();
        return 1;
    }
    populate_global_descriptor_set();


    if (create_present_descriptor_set() != 0)
    {
        std::cout << "Failed to create present descriptor set!" << std::endl;
        destroy();
        return 1;
    }

    if (create_present_pipeline() != 0)
    {
        std::cout << "Failed to create present pipeline!" << std::endl;
        destroy();
        return 1;
    }

    if (create_lighting() != 0)
    {
        std::cout << "Failed to create lighting!" << std::endl;
        destroy();
        return 1;
    }

	if (create_model_pipelines() != 0)
	{
	    std::cout << "Failed to create model pipelines!" << std::endl;
	    destroy();
	    return 1;
	}

    //Helpers
    if (m_Helper_HDR_CubeMaps.create(m_Helper_HDR_CubeMaps_Shader) != 0)
    {
        std::cout << "Failed to create CubeMaps helper!" << std::endl;
        destroy();
        return 1;
    }

	// // bloom.HDRImageView = hdrPass.HDR.getImageView();
	// // bloom.HDRImage = hdrPass.HDR.getImage();
	// // bloom.ClampToEdgeLINEAR = getSampler("ClampToEdgeLINEAR");
	// // bloom.vertexShader = getSystemShaderModule("vertex_default");
	// // bloom.m_FragmentShader = getSystemShaderModule("bloom");
	// // if (bloom.create() != 0)
	// // {
	// // 	std::cout << "Failed to create bloom effect!" << std::endl;
	// // 	return 1;
	// // }
 //

    return 0;
}

void DeferredRendering::Drawing::draw(VkCommandBuffer commandBuffer, unsigned int imageIndex) const
{

    const auto _frame = SmoothieCore::getCurrentFrame();
    begin_command_buffers(_frame);

    Pipeline_Data _pipeline_data;
    _pipeline_data.buffers = m_CurrentDrawingBuffer;
	_pipeline_data.descriptorSet = m_Global_DescriptorSet;
	_pipeline_data.imageIndex = imageIndex;

    const auto height = SmoothieCore::getScrHeight();
    const auto width = SmoothieCore::getScrWidth();

    _pipeline_data.viewport.x = 0.0f;
    _pipeline_data.viewport.y = 0.0f;
    _pipeline_data.viewport.width = static_cast<float>(width);
    _pipeline_data.viewport.height = static_cast<float>(height);
    _pipeline_data.viewport.minDepth = 0.0f;
    _pipeline_data.viewport.maxDepth = 1.0f;

    _pipeline_data.scissor.offset = { 0, 0 };
    _pipeline_data.scissor.extent.width = width;
    _pipeline_data.scissor.extent.height = height;

    {
        std::lock_guard<std::mutex> lock(m_Pipelines_Model_Mutex);
	    for (const auto& [_key, _pipe] : m_Pipelines_Model)
        {
            _pipe->record_data(_pipeline_data);
        }
    }

	// lighting_global.draw(_command_buffers.GP_HDR, m_Global_DescriptorSet, imageIndex);
	// skybox.draw(_command_buffers.GP_HDR, m_Global_DescriptorSet, imageIndex);
    m_Light_Global->draw(m_CurrentDrawingBuffer.GP_HDR, m_Global_DescriptorSet, SmoothieCore::getScrWidth(), SmoothieCore::getScrHeight());
    m_Pipeline_Sky->record_data(_pipeline_data);
    end_command_buffers();

    //Copy-Transfer pass
    m_CameraBuffer.copy_to_gpu(commandBuffer);


    begin_gBuffer(commandBuffer);
	vkCmdExecuteCommands(commandBuffer, 1, &m_CurrentDrawingBuffer.GP_gBuffer);
	end_gBuffer(commandBuffer);
	
	//Screen space lighting effects
	//ssao.draw(commandBuffer, m_Global_DescriptorSet, imageIndex);

	//HDR pass
	begin_hdr(commandBuffer);
	vkCmdExecuteCommands(commandBuffer, 1, &m_CurrentDrawingBuffer.GP_HDR);
	end_hdr(commandBuffer);

	//Post-processing effects
	//bloom.draw(commandBuffer, m_Global_DescriptorSet, imageIndex);

	//If editor class exists, that class will handle the presentation
	if (get_editor_core() != nullptr) return;

	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.renderPass = SmoothieCore::getDefaultRenderPass();
	beginInfo.framebuffer = SmoothieCore::getSwapchainFramebuffer(imageIndex);
	beginInfo.renderArea.offset = { 0, 0 };
	beginInfo.renderArea.extent.height = SmoothieCore::getScrHeight();
	beginInfo.renderArea.extent.width = SmoothieCore::getScrWidth();
	VkClearValue clearValue{};
	clearValue.depthStencil = { 0.0f, 0 };
	clearValue.color.float32[0] = 0.0f;
	clearValue.color.float32[1] = 0.0f;
	clearValue.color.float32[2] = 0.0f;
	clearValue.color.float32[3] = 1.0f;
	beginInfo.clearValueCount = 1;
	beginInfo.pClearValues = &clearValue;
	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PresentPipeline);
	SmoothieCore::setViewport(commandBuffer);
	SmoothieCore::setScissor(commandBuffer);


	const VkDescriptorSet _descriptors[2] =
	{
		m_Global_DescriptorSet,
		m_PresentPipeline_DescriptorSet
	};
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PresentPipeline_Layout, 0, 2, _descriptors, 0, 0);
	vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);
}

int DeferredRendering::Drawing::resize_callback()
{
    destroy_gBuffer();
	if (create_gBuffer() != 0)
	{
		std::cout << "Failed to resize gBuffer pass!" << std::endl;
		return 1;
	}

	destroy_hdr();
	if (create_hdr() != 0)
	{
		std::cout << "Failed to resize HDR pass!" << std::endl;
		return 1;
	}

	// lighting_renderPass.target = hdrPass.HDR;
	// if (lighting_renderPass.resize_callback() != 0)
	// {
	// 	std::cout << "Failed to resize lighting render pass!" << std::endl;
	// 	return 1;
	// }

	// ssao.gDepthImageView = gBuffer_Pass.gDepth.getImageView();
	// ssao.gNormalImageView = gBuffer_Pass.gNormal.getImageView();
	// if (ssao.resize_callback() != 0)
	// {
	// 	std::cout << "Failed to resize SSAO!" << std::endl;
	// 	return 1;
	// }

	// bloom.HDRImageView = hdrPass.HDR.getImageView();
	// bloom.HDRImage = hdrPass.HDR.getImage();
	// if (bloom.resize_callback() != 0)
	// {
	// 	std::cout << "Failed to resize bloom effect!" << std::endl;
	// 	return 1;
	// }

	//Update last render pass
	// std::array<VkImageView, 1> images =
	// {
	// 	bloom.getFinalImage()
	// };
	//
	// for (int i = 0; i < images.size(); i++)
	// {
	// 	VkDescriptorImageInfo imgInfo{};
	// 	imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	// 	imgInfo.imageView = images[i];
	// 	imgInfo.sampler = this->getSampler("Texture2DModelSampler");
	//
	// 	VkWriteDescriptorSet descriptorWrite{};
	// 	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	// 	descriptorWrite.dstSet = m_PresentPipeline_DescriptorSet;
	// 	descriptorWrite.dstArrayElement = 0;
	// 	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	// 	descriptorWrite.descriptorCount = 1;
	// 	descriptorWrite.dstBinding = i;
	// 	descriptorWrite.pBufferInfo = nullptr;
	// 	descriptorWrite.pImageInfo = &imgInfo;
	// 	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
	// }

	return 0;
}

void DeferredRendering::Drawing::destroy()
{
	for (auto& [_key_, _pipe]: m_Pipelines_Model)
	{
		if (_pipe != nullptr) _pipe->destroy();
	}
	m_Pipelines_Model.clear();

	//
	// destroy_present_pipeline();
	// //bloom.destroy();
	// skybox.destroy();
	// lighting_global.destroy();
	// //ssao.destroy();
	// indirect_lighting_maps.destroy();
	// skyboxTexture.destroy();
	// brdf.destroy();
	// lighting_renderPass.destroy();
	//
	//
	// for (auto& _descriptor: descriptors)
	// {
	// 	_descriptor.destroy();
	// }
	// descriptors.clear();


    m_Helper_HDR_CubeMaps.destroy();
    destroy_model_pipelines();
    destroy_lighting();
    destroy_present_pipeline();
    destroy_present_descriptor_set();
    destroy_global_descriptor_sets();
    m_PresentPipeline_ShaderFile.destroy();
    destroy_camera();
    destroy_samplers();
    destroy_hdr();
    destroy_gBuffer();
	destroy_command_buffers();
}


