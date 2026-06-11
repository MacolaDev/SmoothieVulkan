//
// Created by macola on 6/4/26.
//

#include "Sky.h"
#include "Core/SmoothieCore.h"

int Smoothie::DeferredRendering::Sky::create_pipeline_skybox()
{
    const auto& _draw_class = std::dynamic_pointer_cast<DeferredRendering::Drawing>(SmoothieCore::getDrawingClassPtr());
    if (_draw_class == nullptr)
    {
        std::cout << "Failed to cast drawing class to \"DeferredRendering::Drawing\"" << std::endl;
        return 1;
    }

    const VkDescriptorSetLayout _descriptor_set_layouts[] =
    {
        _draw_class->getGlobalDescriptorSetLayout(),
    };

    VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info{};
    vk_pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vk_pipeline_layout_create_info.pNext = nullptr;
    vk_pipeline_layout_create_info.flags = 0;
    vk_pipeline_layout_create_info.setLayoutCount = std::size(_descriptor_set_layouts);
    vk_pipeline_layout_create_info.pSetLayouts = _descriptor_set_layouts;
    vk_pipeline_layout_create_info.pushConstantRangeCount = 0;
    vk_pipeline_layout_create_info.pPushConstantRanges = nullptr;
    if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &vk_pipeline_layout_create_info, nullptr, &m_PipelineLayout_Skybox) != 0)
    {
        std::cout << "Failed to create pipeline layout!" << std::endl;
        return 1;
    }


    VkPipelineShaderStageCreateInfo _shader_stage_vertex{};
    _shader_stage_vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    _shader_stage_vertex.pName  = m_Pipeline_Skybox_VertexShaderEntryName.c_str();
    _shader_stage_vertex.stage = VK_SHADER_STAGE_VERTEX_BIT;
    _shader_stage_vertex.module = m_ShaderFile.get_ShaderModule();

    VkPipelineShaderStageCreateInfo _shader_stage_fragment{};
    _shader_stage_fragment.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    _shader_stage_fragment.pName = m_Pipeline_Skybox_FragmentShaderEntryName.c_str();
    _shader_stage_fragment.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    _shader_stage_fragment.module = m_ShaderFile.get_ShaderModule();

    assert(_shader_stage_vertex.pName != nullptr);
    assert(_shader_stage_vertex.module != nullptr);

    assert(_shader_stage_fragment.pName != nullptr);
    assert(_shader_stage_fragment.module != nullptr);

    VkPipelineDepthStencilStateCreateInfo _depth_stage_create_info{};
    _depth_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    _depth_stage_create_info.pNext = nullptr;
    _depth_stage_create_info.flags = 0;
    _depth_stage_create_info.depthTestEnable = VK_TRUE;
    _depth_stage_create_info.depthWriteEnable = VK_FALSE;
    _depth_stage_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    _depth_stage_create_info.depthBoundsTestEnable = VK_FALSE;
    _depth_stage_create_info.stencilTestEnable = VK_FALSE;
    _depth_stage_create_info.front = {};
    _depth_stage_create_info.back = {};
    _depth_stage_create_info.minDepthBounds = 0.0f;
    _depth_stage_create_info.maxDepthBounds = 1.0f;

    VkPipelineRenderingCreateInfo _rendering_create_info{};
    _draw_class->getRenderingInfo_HDR(_rendering_create_info);

    const VkPipelineShaderStageCreateInfo stages[] = { _shader_stage_vertex, _shader_stage_fragment };
    VkGraphicsPipelineCreateInfo _pipeline_create_info{};
    _draw_class->lazy_populate_graphics_pipeline(_pipeline_create_info);
    _pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    _pipeline_create_info.pNext = &_rendering_create_info;
    _pipeline_create_info.stageCount = 2;
    _pipeline_create_info.pStages = stages;
    _pipeline_create_info.pDepthStencilState = &_depth_stage_create_info;
    _pipeline_create_info.layout = m_PipelineLayout_Skybox;

    assert(m_PipelineLayout_Skybox != nullptr);
    if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), nullptr, 1, &_pipeline_create_info, nullptr, &m_Pipeline_Skybox) != VK_SUCCESS)
    {
        std::cout << "Failed to create pipeline!" << std::endl;
        return 1;
    }

    return 0;
}

void Smoothie::DeferredRendering::Sky::destroy_pipeline_skybox()
{
    if (m_Pipeline_Skybox != nullptr)
    {
        vkDestroyPipeline(SmoothieCore::getDevice(), m_Pipeline_Skybox, nullptr);
        m_Pipeline_Skybox = nullptr;
    }

    if (m_PipelineLayout_Skybox != nullptr)
    {
        vkDestroyPipelineLayout(SmoothieCore::getDevice(), m_PipelineLayout_Skybox, nullptr);
        m_PipelineLayout_Skybox = nullptr;
    }
}

int Smoothie::DeferredRendering::Sky::create(const std::string& shader)
{
    if (m_ShaderFile.create(shader) != 0)
    {
        std::cout << "Failed to create shader!" << std::endl;
        destroy();
        return 1;
    }

    if (create_pipeline_skybox() != 0)
    {
        std::cout << "Failed to create skybox pipeline" << std::endl;
        destroy();
        return 1;
    }

    return 0;
}

int Smoothie::DeferredRendering::Sky::update()
{
    return 0;
}

void Smoothie::DeferredRendering::Sky::record_data(const Pipeline_Data &drawData) const
{
    const auto commandBuffer = drawData.buffers.GP_HDR;

    const VkDescriptorSet _descriptor_sets[] =
    {
        drawData.descriptorSet
    };

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline_Skybox);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout_Skybox, 0, std::size(_descriptor_sets), _descriptor_sets, 0, nullptr);
    vkCmdSetScissor(commandBuffer, 0, 1, &drawData.scissor);
    vkCmdSetViewport(commandBuffer, 0, 1, &drawData.viewport);
    vkCmdDraw(commandBuffer, 36, 1, 0, 0);
}

void Smoothie::DeferredRendering::Sky::destroy()
{
    destroy_pipeline_skybox();
    m_ShaderFile.destroy();
}
