//
// Created by macola on 6/1/26.
//

#include "Light_Global.h"

#include "Core/SmoothieCore.h"
#include "Core/Multithreading.h"

using namespace Smoothie;

static inline void brdf_begin_rendering(VkCommandBuffer commandBuffer, VkImage image, VkImageView view, unsigned int width, unsigned int height)
{
    VkImageMemoryBarrier2 _memory_barrier{};
    _memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _memory_barrier.pNext = nullptr;
    _memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    _memory_barrier.srcAccessMask = VK_ACCESS_2_NONE;
    _memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    _memory_barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    _memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _memory_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _memory_barrier.image = image;
    _memory_barrier.subresourceRange.layerCount = 1;
    _memory_barrier.subresourceRange.levelCount = 1;
    _memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;


    VkDependencyInfo _dependency{};
    _dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    _dependency.imageMemoryBarrierCount = 1;
    _dependency.pImageMemoryBarriers = &_memory_barrier;
    vkCmdPipelineBarrier2(commandBuffer, &_dependency);

    VkRenderingAttachmentInfo _color_rendering_attachment{};
    _color_rendering_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    _color_rendering_attachment.pNext = nullptr;
    _color_rendering_attachment.imageView = view;
    _color_rendering_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _color_rendering_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    _color_rendering_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    _color_rendering_attachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};

    VkRenderingInfo _rendering_info{};
    _rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    _rendering_info.pNext = nullptr;
    _rendering_info.flags = 0;
    _rendering_info.renderArea.offset = { 0, 0 };
    _rendering_info.renderArea.extent.width = width;
    _rendering_info.renderArea.extent.height = height;
    _rendering_info.layerCount = 1;
    _rendering_info.viewMask = 0;
    _rendering_info.colorAttachmentCount = 1;
    _rendering_info.pColorAttachments = &_color_rendering_attachment;
    vkCmdBeginRendering(commandBuffer, &_rendering_info);
}

static inline void brdf_end_rendering(VkCommandBuffer commandBuffer, VkImage image, VkImageView view)
{
    vkCmdEndRendering(commandBuffer);

    VkImageMemoryBarrier2 _memory_barrier{};
    _memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _memory_barrier.pNext = nullptr;
    _memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    _memory_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    _memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    _memory_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    _memory_barrier.image = image;
    _memory_barrier.subresourceRange.layerCount = 1;
    _memory_barrier.subresourceRange.levelCount = 1;
    _memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkDependencyInfo _dependency{};
    _dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    _dependency.imageMemoryBarrierCount = 1;
    _dependency.pImageMemoryBarriers = &_memory_barrier;
    vkCmdPipelineBarrier2(commandBuffer, &_dependency);

}

int Smoothie::DeferredRendering::Light_Global::create_brdf()
{
    const auto& _draw_class = std::reinterpret_pointer_cast<DeferredRendering::Drawing>(SmoothieCore::getDrawingClassPtr());
    if (_draw_class == nullptr)
    {
        std::cout << "Failed to cast drawing class to \"DeferredRendering::Drawing\" class!!" << std::endl;
        return 1;
    }

    VkImageCreateInfo _image_create_info_brdf{};
    _image_create_info_brdf.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    _image_create_info_brdf.imageType = VK_IMAGE_TYPE_2D;
    _image_create_info_brdf.extent = m_Extent_BRDF;
    _image_create_info_brdf.mipLevels = 1;
    _image_create_info_brdf.arrayLayers = 1;
    _image_create_info_brdf.tiling = VK_IMAGE_TILING_OPTIMAL;
    _image_create_info_brdf.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _image_create_info_brdf.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    _image_create_info_brdf.samples = VK_SAMPLE_COUNT_1_BIT;
    _image_create_info_brdf.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    _image_create_info_brdf.format = m_Format_BRDF;

    VmaAllocationCreateInfo _image_alloc_info_brdf = {};
    _image_alloc_info_brdf.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    _image_alloc_info_brdf.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    _image_alloc_info_brdf.priority = 1.0f;
    if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_image_create_info_brdf, &_image_alloc_info_brdf, &m_Image_BRDF, &m_Allocation_BRDF, nullptr) != VK_SUCCESS)
    {
        std::cout << "Failed to create image!" << std::endl;
        return 1;
    }

    VkImageViewCreateInfo _imageView_create_info_brdf{};
    _imageView_create_info_brdf.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    _imageView_create_info_brdf.image = m_Image_BRDF;
    _imageView_create_info_brdf.viewType = VK_IMAGE_VIEW_TYPE_2D;
    _imageView_create_info_brdf.format = _image_create_info_brdf.format;
    _imageView_create_info_brdf.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _imageView_create_info_brdf.subresourceRange.levelCount = 1;
    _imageView_create_info_brdf.subresourceRange.layerCount = 1;
    if (vkCreateImageView(SmoothieCore::getDevice(), &_imageView_create_info_brdf, nullptr, &m_ImageView_BRDF) != VK_SUCCESS)
    {
        std::cout << "Failed to create image view!" << std::endl;
        return 1;
    }


    VkPipelineLayout _brdf_pipeline_layout = nullptr;
    VkPipelineLayoutCreateInfo _brdf_pipeline_layout_create_info{};
    _brdf_pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    _brdf_pipeline_layout_create_info.setLayoutCount = 0;
    _brdf_pipeline_layout_create_info.pSetLayouts = nullptr;
    if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &_brdf_pipeline_layout_create_info, nullptr, &_brdf_pipeline_layout) != 0)
    {
        std::cout << "Failed to create brdg pipeline layout!" << std::endl;
        return 1;
    }

    VkPipelineShaderStageCreateInfo _fragment_shader_stage_create_info{};
    _fragment_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    _fragment_shader_stage_create_info.pNext = nullptr;
    _fragment_shader_stage_create_info.flags = 0;
    _fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    _fragment_shader_stage_create_info.module = m_ShaderFile.get_ShaderModule();
    assert(_fragment_shader_stage_create_info.module != nullptr);
    _fragment_shader_stage_create_info.pName = m_ShaderEntryName_BRDF.c_str();
    _fragment_shader_stage_create_info.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo _vertex_shader_stage_create_info{};
    _vertex_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    _vertex_shader_stage_create_info.pNext = nullptr;
    _vertex_shader_stage_create_info.flags = 0;
    _vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    _vertex_shader_stage_create_info.module = _draw_class->getPresentPipelineShaderModule();
    assert(_vertex_shader_stage_create_info.module != nullptr);
    _vertex_shader_stage_create_info.pName = _draw_class->getPresentPipelineVertexShaderEntryName().c_str();
    _vertex_shader_stage_create_info.pSpecializationInfo = nullptr;

    const VkPipelineShaderStageCreateInfo _shader_stage_create_infos[] = {_vertex_shader_stage_create_info, _fragment_shader_stage_create_info};

    const VkFormat _attachment_formats[] = {m_Format_BRDF};
    VkPipelineRenderingCreateInfo _pipeline_rendering_info{};
    _pipeline_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    _pipeline_rendering_info.pNext = nullptr;
    _pipeline_rendering_info.viewMask = 0;
    _pipeline_rendering_info.colorAttachmentCount = std::size(_attachment_formats);
    _pipeline_rendering_info.pColorAttachmentFormats = _attachment_formats;
    _pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    _pipeline_rendering_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    VkGraphicsPipelineCreateInfo _brdf_pipeline_create_info{};
    _draw_class->lazy_populate_graphics_pipeline(_brdf_pipeline_create_info);
    _brdf_pipeline_create_info.pNext = &_pipeline_rendering_info;
    _brdf_pipeline_create_info.stageCount = std::size(_shader_stage_create_infos);
    _brdf_pipeline_create_info.pStages = _shader_stage_create_infos;
    _brdf_pipeline_create_info.layout = _brdf_pipeline_layout;

    VkPipeline _brdf_pipeline = nullptr;
    if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), nullptr, 1, &_brdf_pipeline_create_info, nullptr, &_brdf_pipeline) != VK_SUCCESS)
    {

        if (_brdf_pipeline != nullptr) {vkDestroyPipeline(SmoothieCore::getDevice(), _brdf_pipeline, nullptr);}
        if (_brdf_pipeline_layout != nullptr) {vkDestroyPipelineLayout(SmoothieCore::getDevice(), _brdf_pipeline_layout, nullptr);}

        std::cout << "Failed to create brdf pipeline!" << std::endl;
        return 1;
    }


    ImmediateCommandBuffer _immediateCommandBuffer;
    _immediateCommandBuffer.create();
    _immediateCommandBuffer.begin();
    auto commandBuffer = _immediateCommandBuffer.get_CommandBuffer();

    brdf_begin_rendering(commandBuffer, m_Image_BRDF, m_ImageView_BRDF, m_Extent_BRDF.width, m_Extent_BRDF.height);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _brdf_pipeline);
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_Extent_BRDF.width);
    viewport.height = static_cast<float>(m_Extent_BRDF.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent.height = m_Extent_BRDF.height;
    scissor.extent.width = m_Extent_BRDF.width;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);

    brdf_end_rendering(commandBuffer, m_Image_BRDF, m_ImageView_BRDF);

    _immediateCommandBuffer.end();
    _immediateCommandBuffer.submitAndWait();
    _immediateCommandBuffer.destroy();


    vkDestroyPipeline(SmoothieCore::getDevice(), _brdf_pipeline, nullptr);
    vkDestroyPipelineLayout(SmoothieCore::getDevice(), _brdf_pipeline_layout, nullptr);
    return 0;
}

void Smoothie::DeferredRendering::Light_Global::destroy_brdf()
{
    if (m_ImageView_BRDF != nullptr)
    {
        vkDestroyImageView(SmoothieCore::getDevice(), m_ImageView_BRDF, nullptr);
        m_ImageView_BRDF = nullptr;
    }

    if (m_Image_BRDF != nullptr)
    {
        vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_Image_BRDF, m_Allocation_BRDF);
        m_Image_BRDF = nullptr;
        m_Allocation_BRDF = nullptr;
    }

}

int DeferredRendering::Light_Global::create_irradiance()
{

	VkImageCreateInfo _image_create_info_irradiance{};
	_image_create_info_irradiance.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	_image_create_info_irradiance.imageType = VK_IMAGE_TYPE_2D;
	_image_create_info_irradiance.extent = m_Extent_Irradiance;
	_image_create_info_irradiance.mipLevels = 1;
	_image_create_info_irradiance.arrayLayers = 6;
	_image_create_info_irradiance.tiling = VK_IMAGE_TILING_OPTIMAL;
	_image_create_info_irradiance.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_image_create_info_irradiance.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	_image_create_info_irradiance.samples = VK_SAMPLE_COUNT_1_BIT;
	_image_create_info_irradiance.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	_image_create_info_irradiance.format = m_Format_Irradiance;
	_image_create_info_irradiance.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	VmaAllocationCreateInfo _allocation_info = {};
	_allocation_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	_allocation_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	_allocation_info.priority = 1.0f;
	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_image_create_info_irradiance, &_allocation_info, &m_Image_Irradiance, &m_Allocation_Irradiance, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create irradiance map image!" << std::endl;
		return 1;
	}

	VkImageViewCreateInfo _image_view_create_info_irradiance{};
	_image_view_create_info_irradiance.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	_image_view_create_info_irradiance.image = m_Image_Irradiance;
	_image_view_create_info_irradiance.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	_image_view_create_info_irradiance.format = _image_create_info_irradiance.format;
	_image_view_create_info_irradiance.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	_image_view_create_info_irradiance.subresourceRange.baseMipLevel = 0;
	_image_view_create_info_irradiance.subresourceRange.levelCount = 1;
	_image_view_create_info_irradiance.subresourceRange.baseArrayLayer = 0;
	_image_view_create_info_irradiance.subresourceRange.layerCount = 6;
	if (vkCreateImageView(SmoothieCore::getDevice(), &_image_view_create_info_irradiance, nullptr, &m_ImageView_Irradiance) != VK_SUCCESS)
	{
		std::cout << "Failed to create image map image view!" << std::endl;
		return 1;
	}

	ImmediateCommandBuffer _immediateCommandBuffer;
	_immediateCommandBuffer.create();
	_immediateCommandBuffer.begin();
	auto commandBuffer = _immediateCommandBuffer.get_CommandBuffer();

    VkImageMemoryBarrier2 _memory_barrier{};
    _memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _memory_barrier.pNext = nullptr;
    _memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    _memory_barrier.srcAccessMask = VK_ACCESS_2_NONE;
    _memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    _memory_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    _memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    _memory_barrier.image = m_Image_Irradiance;
    _memory_barrier.subresourceRange.layerCount = 6;
    _memory_barrier.subresourceRange.levelCount = 1;
    _memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkDependencyInfo _dependency{};
    _dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    _dependency.imageMemoryBarrierCount = 1;
    _dependency.pImageMemoryBarriers = &_memory_barrier;
    vkCmdPipelineBarrier2(commandBuffer, &_dependency);

    constexpr VkClearColorValue _clear_color_value{0.0f, 0.0f, 0.0f, 1.0f};
	vkCmdClearColorImage(commandBuffer, m_Image_Irradiance, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &_clear_color_value, 1, &_image_view_create_info_irradiance.subresourceRange);

    _memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    _memory_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    _memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _memory_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    _memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    _memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkCmdPipelineBarrier2(commandBuffer, &_dependency);

	_immediateCommandBuffer.end();
	_immediateCommandBuffer.submitAndWait();
    _immediateCommandBuffer.destroy();
    return 0;
}

void DeferredRendering::Light_Global::destroy_irradiance()
{
    if (m_ImageView_Irradiance != nullptr)
    {
        vkDestroyImageView(SmoothieCore::getDevice(), m_ImageView_Irradiance, nullptr);
        m_ImageView_Irradiance = nullptr;
    }

    if (m_Image_Irradiance != nullptr)
    {
        vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_Image_Irradiance, m_Allocation_Irradiance);
        m_Image_Irradiance = nullptr, m_Allocation_Irradiance = nullptr;
    }

}

int DeferredRendering::Light_Global::create_prefiler()
{
    VkImageCreateInfo _image_create_info_prefilter{};
	_image_create_info_prefilter.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	_image_create_info_prefilter.imageType = VK_IMAGE_TYPE_2D;
	_image_create_info_prefilter.extent = m_Extent_Prefilter;
	_image_create_info_prefilter.mipLevels = m_MipLevels_Prefilter;
	_image_create_info_prefilter.arrayLayers = 6;
	_image_create_info_prefilter.tiling = VK_IMAGE_TILING_OPTIMAL;
	_image_create_info_prefilter.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_image_create_info_prefilter.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	_image_create_info_prefilter.samples = VK_SAMPLE_COUNT_1_BIT;
	_image_create_info_prefilter.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	_image_create_info_prefilter.format = m_Format_Prefilter;
	_image_create_info_prefilter.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VmaAllocationCreateInfo _allocation_info = {};
    _allocation_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    _allocation_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    _allocation_info.priority = 1.0f;

	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_image_create_info_prefilter, &_allocation_info, &m_Image_Prefilter, &m_Allocation_Prefilter, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create prefilter image!" << std::endl;
		return 1;
	}

	VkImageViewCreateInfo _image_view_create_info{};
	_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	_image_view_create_info.image = m_Image_Prefilter;
	_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	_image_view_create_info.format = m_Format_Prefilter;
	_image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	_image_view_create_info.subresourceRange.baseMipLevel = 0;
	_image_view_create_info.subresourceRange.levelCount = 5;
	_image_view_create_info.subresourceRange.baseArrayLayer = 0;
	_image_view_create_info.subresourceRange.layerCount = 6;
	if (vkCreateImageView(SmoothieCore::getDevice(), &_image_view_create_info, nullptr, &m_ImageView_Prefilter) != VK_SUCCESS)
	{
		std::cout << "Failed to create prefilter map image view!" << std::endl;
		return 1;
	}


    ImmediateCommandBuffer _immediateCommandBuffer;
	_immediateCommandBuffer.create();
	_immediateCommandBuffer.begin();
	auto commandBuffer = _immediateCommandBuffer.get_CommandBuffer();

    VkImageMemoryBarrier2 _memory_barrier{};
    _memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _memory_barrier.pNext = nullptr;
    _memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    _memory_barrier.srcAccessMask = VK_ACCESS_2_NONE;
    _memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    _memory_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    _memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    _memory_barrier.image = m_Image_Prefilter;
    _memory_barrier.subresourceRange.layerCount = 6;
    _memory_barrier.subresourceRange.levelCount = m_MipLevels_Prefilter;
    _memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkDependencyInfo _dependency{};
    _dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    _dependency.imageMemoryBarrierCount = 1;
    _dependency.pImageMemoryBarriers = &_memory_barrier;
    vkCmdPipelineBarrier2(commandBuffer, &_dependency);

    constexpr VkClearColorValue _clear_color_value{0.0f, 0.0f, 0.0f, 1.0f};
	vkCmdClearColorImage(commandBuffer, m_Image_Prefilter, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &_clear_color_value, 1, &_image_view_create_info.subresourceRange);

    _memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    _memory_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    _memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _memory_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    _memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    _memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkCmdPipelineBarrier2(commandBuffer, &_dependency);

	_immediateCommandBuffer.end();
	_immediateCommandBuffer.submitAndWait();
    _immediateCommandBuffer.destroy();

    return 0;
}

void DeferredRendering::Light_Global::destroy_prefiler()
{
    if (m_ImageView_Prefilter != nullptr)
    {
        vkDestroyImageView(SmoothieCore::getDevice(), m_ImageView_Prefilter, nullptr);
        m_ImageView_Prefilter = nullptr;
    }

    if (m_Image_Prefilter!= nullptr)
    {
        vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_Image_Prefilter, m_Allocation_Prefilter);
        m_Image_Prefilter = nullptr, m_Allocation_Prefilter = nullptr;
    }
}

int DeferredRendering::Light_Global::create_descriptors()
{
    std::vector<VkDescriptorSetLayoutBinding> _bindings;
    std::vector<VkDescriptorPoolSize> _pool_sizes;
    if (m_ShaderFile.get_DescriptorData(m_DescriptorSet_Name, _bindings, _pool_sizes) != 0)
    {
        std::cout << "Failed to get descriptor data!" << std::endl;
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
        std::cout << "Descriptor set is not supported!" << std::endl;
        return 1;
    }

    if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &_layoutCreateInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
    {
        std::cout << "Failed to create descriptor set!" << std::endl;
        return 1;
    }

    VkDescriptorPoolCreateInfo _poolCreateInfo{};
    _poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    _poolCreateInfo.pNext = nullptr;
    _poolCreateInfo.poolSizeCount = static_cast<unsigned int>(_pool_sizes.size());
    _poolCreateInfo.pPoolSizes = _pool_sizes.data();
    _poolCreateInfo.maxSets = 1;
    if (vkCreateDescriptorPool(SmoothieCore::getDevice(), &_poolCreateInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
    {
        std::cout << "Failed to create descriptor pool!" << std::endl;
        return 1;
    }

    VkDescriptorSetAllocateInfo _allocInfo = {};
    _allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    _allocInfo.pNext = nullptr;
    _allocInfo.descriptorPool = m_DescriptorPool;
    _allocInfo.descriptorSetCount = 1;
    _allocInfo.pSetLayouts = &m_DescriptorSetLayout;
    if (vkAllocateDescriptorSets(SmoothieCore::getDevice(), &_allocInfo, &m_DescriptorSet) != VK_SUCCESS)
    {
        std::cout << "Failed to create descriptor set!" << std::endl;
        return 1;
    }

    std::unordered_map<std::string, WriteData_Descriptor> _layoutBindings;
    std::unordered_map<std::string, WriteData_Buffer> _buffers;
    if (m_ShaderFile.get_DescriptorData(m_DescriptorSet_Name, _layoutBindings, _buffers) != 0)
    {
        std::cout << "Failed to get descriptor data!" << std::endl;
        return 1;
    }


    const std::unordered_map<std::string, VkImageView> _image_views =
    {
        {m_DescriptorSet_Name + "." + m_DescriptorSetName_BRDF, m_ImageView_BRDF},
        {m_DescriptorSet_Name + "." + m_DescriptorSetName_Irradiance, m_ImageView_Irradiance},
        {m_DescriptorSet_Name + "." + m_DescriptorSetName_Prefilter, m_ImageView_Prefilter}
    };


    for (const auto& [_path, _descriptor] : _layoutBindings)
    {
        if (_descriptor.type != VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) {continue;}

        VkDescriptorImageInfo _image_info{};
        _image_info.imageView = m_ImageView_BRDF;
        _image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        if (_image_views.find(_path) == _image_views.end())
        {
            std::cout << "Failed to find image view \"" << _path << "\" in current map! Attempting to use default values!" << std::endl;
            _image_info.imageView = SmoothieCore::getDefault2DTexture().getImageView();

        }else
        {
            _image_info.imageView = _image_views.at(_path);
        }

        VkWriteDescriptorSet _write_descriptor_set = {};
        _write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        _write_descriptor_set.pNext = nullptr;
        _write_descriptor_set.dstSet = m_DescriptorSet;
        _write_descriptor_set.dstBinding = _descriptor.binding;
        _write_descriptor_set.descriptorCount = 1;
        _write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        _write_descriptor_set.pImageInfo = &_image_info;
        vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_write_descriptor_set, 0, nullptr);
    }


    return 0;
}

void DeferredRendering::Light_Global::destroy_descriptors()
{
    if (m_DescriptorPool != nullptr)
    {
        vkDestroyDescriptorPool(SmoothieCore::getDevice(), m_DescriptorPool, nullptr);
        m_DescriptorPool = nullptr;
        m_DescriptorSet = nullptr;
    }

    if (m_DescriptorSetLayout != nullptr)
    {
        vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), m_DescriptorSetLayout, nullptr);
        m_DescriptorSetLayout = nullptr;
    }
}

int DeferredRendering::Light_Global::create_pipeline()
{
    const auto& _drawing_class = std::dynamic_pointer_cast<DeferredRendering::Drawing>(SmoothieCore::getDrawingClassPtr());
    if (_drawing_class == nullptr)
    {
        std::cout << "Failed to cast drawing class to \"DeferredRendering::Drawing\"!" << std::endl;
        return 1;
    }

    const VkDescriptorSetLayout _descriptorSetLayouts[] =
    {
        _drawing_class->getGlobalDescriptorSetLayout(),
        m_DescriptorSetLayout
    };

    VkPipelineLayoutCreateInfo _pipeline_layout_createInfo{};
    _pipeline_layout_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    _pipeline_layout_createInfo.setLayoutCount = std::size(_descriptorSetLayouts);
    _pipeline_layout_createInfo.pSetLayouts = _descriptorSetLayouts;
    if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &_pipeline_layout_createInfo, nullptr, &m_PipelineLayout) != 0)
    {
        std::cout << "Failed to create pipeline layout!" << std::endl;
        return 1;
    }

    VkPipelineShaderStageCreateInfo _shader_stage_vertex{};
    _shader_stage_vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    _shader_stage_vertex.pName = _drawing_class->getPresentPipelineVertexShaderEntryName().c_str();
    _shader_stage_vertex.stage = VK_SHADER_STAGE_VERTEX_BIT;
    _shader_stage_vertex.module = _drawing_class->getPresentPipelineShaderModule();

    assert(_shader_stage_vertex.pName != nullptr);
    assert(_shader_stage_vertex.module != nullptr);

    VkPipelineShaderStageCreateInfo _shader_stage_fragment{};
    _shader_stage_fragment.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    _shader_stage_fragment.pName = m_ShaderEntryName_Light.c_str();
    _shader_stage_fragment.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    _shader_stage_fragment.module = m_ShaderFile.get_ShaderModule();

    assert(_shader_stage_fragment.pName != nullptr);
    assert(_shader_stage_fragment.module != nullptr);

    const VkPipelineShaderStageCreateInfo _shader_stages[] = { _shader_stage_vertex, _shader_stage_fragment };

    VkPipelineRenderingCreateInfo _rendering_create_info{};
    _rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    _rendering_create_info.pNext = nullptr;
    _rendering_create_info.colorAttachmentCount = 1;
    _drawing_class->getRenderingInfo_HDR(_rendering_create_info);

    VkGraphicsPipelineCreateInfo _pipeline_createInfo{};
    _drawing_class->lazy_populate_graphics_pipeline(_pipeline_createInfo);
    _pipeline_createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    _pipeline_createInfo.pNext = &_rendering_create_info;
    _pipeline_createInfo.stageCount = std::size(_shader_stages);
    _pipeline_createInfo.pStages = _shader_stages;
    _pipeline_createInfo.pTessellationState = nullptr;
    _pipeline_createInfo.layout = m_PipelineLayout;

    assert(_pipeline_createInfo.layout != nullptr);

    if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), nullptr, 1, &_pipeline_createInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
    {
        std::cout << "Failed to create global illumination pipeline!" << std::endl;
        return 1;
    }

    return 0;
}

void DeferredRendering::Light_Global::destroy_pipeline()
{
    if (m_Pipeline != nullptr)
    {
        vkDestroyPipeline(SmoothieCore::getDevice(), m_Pipeline, nullptr);
        m_Pipeline = nullptr;
    }

    if (m_PipelineLayout != nullptr)
    {
        vkDestroyPipelineLayout(SmoothieCore::getDevice(), m_PipelineLayout, nullptr);
        m_PipelineLayout = nullptr;
    }

}

int Smoothie::DeferredRendering::Light_Global::create(const std::string &shader)
{

    if (m_ShaderFile.create(shader) != 0)
    {
        std::cout << "Failed to create shader file!" << std::endl;
        destroy();
        return 1;
    }

    if (create_brdf() != 0)
    {
        std::cout << "Failed to create brdf data!" << std::endl;
        destroy();
        return 1;
    }

    if (create_irradiance() != 0)
    {
        std::cout << "Failed to create irradiance data!" << std::endl;
        destroy();
        return 1;
    }

    if (create_prefiler() != 0)
    {
        std::cout << "Failed to create prefiler data!" << std::endl;
        destroy();
        return 1;
    }

    if (create_descriptors() != 0)
    {
        std::cout << "Failed to create descriptors!" << std::endl;
        destroy();
        return 1;
    }

    if (create_pipeline() != 0)
    {
        std::cout << "Failed to create pipeline!" << std::endl;
        destroy();
        return 1;
    }


    return 0;
}

void Smoothie::DeferredRendering::Light_Global::draw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, unsigned int width, unsigned int height) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

    const VkDescriptorSet _descriptor_sets[] = { descriptorSet, m_DescriptorSet };
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, std::size(_descriptor_sets), _descriptor_sets, 0, nullptr);

    VkViewport _viewport{};
    _viewport.x = 0.0f;
    _viewport.y = 0.0f;
    _viewport.width = static_cast<float>(width);
    _viewport.height = static_cast<float>(height);
    _viewport.minDepth = 0.0f;
    _viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &_viewport);

    VkRect2D _scissor{};
    _scissor.offset = { 0, 0 };
    _scissor.extent.width = width;
    _scissor.extent.height = height;
    vkCmdSetScissor(commandBuffer, 0, 1, &_scissor);

    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
}

void Smoothie::DeferredRendering::Light_Global::destroy()
{
    destroy_pipeline();
    destroy_descriptors();
    destroy_prefiler();
    destroy_irradiance();
    destroy_brdf();
    m_ShaderFile.destroy();
}
