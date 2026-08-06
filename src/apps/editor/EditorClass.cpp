//
// Created by macola on 7/21/26.
//

#include "EditorClass.h"

#include "backends/imgui_impl_glfw.h"

void EditorClass::compute_post_processing(VkCommandBuffer command_buffer) const
{
    Drawing::compute_post_processing(command_buffer);

    begin_target_pass(command_buffer);
    Drawing::present_pass(command_buffer);
    end_target_pass(command_buffer);

    m_Editor.compute_work(command_buffer);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    m_Editor.ui();
}

void EditorClass::present_pass(VkCommandBuffer command_buffer) const
{
    m_Editor.draw_ui(command_buffer); //ImGui will draw everything now
}

int EditorClass::create()
{
    m_PresentPipeline_RenderingFormat = VK_FORMAT_R8G8B8A8_UNORM;

    if (Drawing::create() != 0)
    {
        SMOOTHIE_CRITICAL_WITH_SRC("Failed to create main drawing class.");
        return 1;
    }

    if(m_RenderTarget.create(
        {SmoothieCore::getScrWidth(), SmoothieCore::getScrHeight(), 1},
        m_PresentPipeline_RenderingFormat,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT) != 0)
    {
        SMOOTHIE_CRITICAL_WITH_SRC("Failed to create render target.");
    }

   m_Editor.set_ResultImage(m_RenderTarget.getImage());
   m_Editor.set_ResultImageView(m_RenderTarget.getImageView());
   m_Editor.set_DepthImage(m_Depth.getImage());
   m_Editor.set_DepthImageView(m_Depth.getImageView());
    if (m_Editor.create() != 0)
    {
        SMOOTHIE_CRITICAL_WITH_SRC("Failed to create editor class.");
    }
    return 0;
}

int EditorClass::resize_callback()
{
    if (Drawing::resize_callback() != 0) return 1;

    m_RenderTarget.destroy();
    if (m_RenderTarget.create({SmoothieCore::getScrWidth(), SmoothieCore::getScrHeight(), 1},
        m_PresentPipeline_RenderingFormat,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT))
    {
        SMOOTHIE_CRITICAL_WITH_SRC("Failed to create render target.");
    }


    m_Editor.set_ResultImage(m_RenderTarget.getImage());
    m_Editor.set_ResultImageView(m_RenderTarget.getImageView());
    m_Editor.set_DepthImage(m_Depth.getImage());
    m_Editor.set_DepthImageView(m_Depth.getImageView());
    if (m_Editor.resize_callback() != 0)
    {
        SMOOTHIE_CRITICAL_WITH_SRC("Failed to re-size the editor.");
    }

    return 0;
}

void EditorClass::destroy()
{
    m_Editor.destroy();
    m_RenderTarget.destroy();
    Drawing::destroy();
}

void EditorClass::begin_target_pass(VkCommandBuffer command_buffer) const
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
    _barrier_color.image = m_RenderTarget.getImage();
    _barrier_color.subresourceRange = _color_subresource;

    VkDependencyInfo _dependency = {};
    _dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    _dependency.pNext = nullptr;
    _dependency.dependencyFlags = 0;
    _dependency.memoryBarrierCount = 0;
    _dependency.pMemoryBarriers = nullptr;
    _dependency.bufferMemoryBarrierCount = 0;
    _dependency.pBufferMemoryBarriers = nullptr;
    _dependency.imageMemoryBarrierCount = 1;
    _dependency.pImageMemoryBarriers = &_barrier_color;
    vkCmdPipelineBarrier2(command_buffer, &_dependency);

    VkRenderingAttachmentInfo _attachment{};
    _attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    _attachment.imageView = m_RenderTarget.getImageView();
    _attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    _attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    _attachment.clearValue.color.float32[0] = 0.0f;
    _attachment.clearValue.color.float32[1] = 0.0f;
    _attachment.clearValue.color.float32[2] = 0.0f;
    _attachment.clearValue.color.float32[3] = 1.0f;

    VkRenderingInfo _rendering_info{};
    _rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    _rendering_info.pNext = nullptr;
    _rendering_info.renderArea.extent = {SmoothieCore::getScrWidth() , SmoothieCore::getScrHeight()};
    _rendering_info.renderArea.offset = {0, 0};
    _rendering_info.layerCount = 1;
    _rendering_info.viewMask = 0;
    _rendering_info.colorAttachmentCount = 1;
    _rendering_info.pColorAttachments = &_attachment;
    vkCmdBeginRendering(command_buffer, &_rendering_info);
}

void EditorClass::end_target_pass(VkCommandBuffer command_buffer) const
{
    vkCmdEndRendering(command_buffer);

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
    _barrier_color.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _barrier_color.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    _barrier_color.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _barrier_color.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    _barrier_color.srcQueueFamilyIndex = 0;
    _barrier_color.dstQueueFamilyIndex = 0;
    _barrier_color.image = m_RenderTarget.getImage();
    _barrier_color.subresourceRange = _color_subresource;

    VkDependencyInfo _dependency = {};
    _dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    _dependency.pNext = nullptr;
    _dependency.dependencyFlags = 0;
    _dependency.memoryBarrierCount = 0;
    _dependency.pMemoryBarriers = nullptr;
    _dependency.bufferMemoryBarrierCount = 0;
    _dependency.pBufferMemoryBarriers = nullptr;
    _dependency.imageMemoryBarrierCount = 1;
    _dependency.pImageMemoryBarriers = &_barrier_color;
    vkCmdPipelineBarrier2(command_buffer, &_dependency);
}
