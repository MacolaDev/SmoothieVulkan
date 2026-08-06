//
// Created by macola on 6/4/26.
//

#include "Sky.h"
#include "Core/SmoothieCore.h"
#include "Core/Multithreading.h"
#include "Effects/Deferred_Core.h"

int Smoothie::DeferredRendering::Sky::create_image_sky()
{
    VkImageCreateInfo _sky_image{};
	_sky_image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	_sky_image.imageType = VK_IMAGE_TYPE_2D;
	_sky_image.extent = m_Extent_Sky;
	_sky_image.mipLevels = 1;
	_sky_image.arrayLayers = 6;
	_sky_image.tiling = VK_IMAGE_TILING_OPTIMAL;
	_sky_image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_sky_image.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	_sky_image.samples = VK_SAMPLE_COUNT_1_BIT;
	_sky_image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	_sky_image.format = m_Format_Sky;
	_sky_image.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	VmaAllocationCreateInfo _allocation_info = {};
	_allocation_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	_allocation_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	_allocation_info.priority = 1.0f;
	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_sky_image, &_allocation_info, &m_Image_Sky, &m_Allocation_Image_Sky, nullptr) != VK_SUCCESS)
	{
	    SMOOTHIE_ERROR_WITH_SRC("Failed to create image.");
		return 1;
	}

	VkImageViewCreateInfo _sky_image_view{};
	_sky_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	_sky_image_view.image = m_Image_Sky;
	_sky_image_view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	_sky_image_view.format = _sky_image.format;
	_sky_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	_sky_image_view.subresourceRange.baseMipLevel = 0;
	_sky_image_view.subresourceRange.levelCount = 1;
	_sky_image_view.subresourceRange.baseArrayLayer = 0;
	_sky_image_view.subresourceRange.layerCount = 6;
	if (vkCreateImageView(SmoothieCore::getDevice(), &_sky_image_view, nullptr, &m_ImageView_Sky) != VK_SUCCESS)
	{
	    SMOOTHIE_ERROR_WITH_SRC("Failed to create image view.");
		return 1;
	}


	ImmediateCommandBuffer _immediateCommandBuffer;
	_immediateCommandBuffer.create();
	_immediateCommandBuffer.begin();
	auto _commandBuffer = _immediateCommandBuffer.get_CommandBuffer();

    VkImageMemoryBarrier2 _barrier{};
    _barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    _barrier.srcAccessMask = VK_ACCESS_2_NONE;
    _barrier.dstStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT;
    _barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    _barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    _barrier.image = m_Image_Sky;
    _barrier.subresourceRange.layerCount = 6;
    _barrier.subresourceRange.levelCount = 1;
    _barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkDependencyInfo _dependency{};
    _dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    _dependency.imageMemoryBarrierCount = 1;
    _dependency.pImageMemoryBarriers = &_barrier;
    vkCmdPipelineBarrier2(_commandBuffer, &_dependency);


	VkClearColorValue _clear_color{};
	_clear_color.float32[0] = 0.69f;
	_clear_color.float32[1] = 0.69f;
	_clear_color.float32[2] = 0.69f;
	_clear_color.float32[3] = 1.0f;

	vkCmdClearColorImage(_commandBuffer, m_Image_Sky, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &_clear_color, 1, &_sky_image_view.subresourceRange);

    _barrier.srcStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT;
    _barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    _barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    _barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    _barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkCmdPipelineBarrier2(_commandBuffer, &_dependency);


	_immediateCommandBuffer.end();
	_immediateCommandBuffer.submitAndWait();
	_immediateCommandBuffer.destroy();

    return 0;
}

void Smoothie::DeferredRendering::Sky::destroy_image_sky()
{
    if (m_ImageView_Sky != nullptr)
    {
        vkDestroyImageView(SmoothieCore::getDevice(), m_ImageView_Sky, nullptr);
        m_ImageView_Sky = nullptr;
    }

    if (m_Image_Sky != nullptr)
    {
        vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_Image_Sky, m_Allocation_Image_Sky);
        m_Image_Sky = nullptr;
        m_Allocation_Image_Sky = nullptr;
    }

}

int Smoothie::DeferredRendering::Sky::create_descriptor_set()
{

    std::vector<VkDescriptorSetLayoutBinding> _bindings;
    std::vector<VkDescriptorPoolSize> _pool_sizes;
    if (m_ShaderFile.get_DescriptorData(m_DescriptorSet_Name, _bindings, _pool_sizes) != 0)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to get descriptor data.");
        return 1;
    }


    VkDescriptorSetLayoutCreateInfo _descriptor_set_layout_create_info{};
    _descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    _descriptor_set_layout_create_info.pNext = nullptr;
    _descriptor_set_layout_create_info.flags = 0;
    _descriptor_set_layout_create_info.bindingCount = _bindings.size();
    _descriptor_set_layout_create_info.pBindings = _bindings.data();
    if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &_descriptor_set_layout_create_info, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create descriptor layout.");
        return 1;
    }

    VkDescriptorPoolCreateInfo _descriptor_pool_create_info{};
    _descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    _descriptor_pool_create_info.pNext = nullptr;
    _descriptor_pool_create_info.flags = 0;
    _descriptor_pool_create_info.poolSizeCount = _pool_sizes.size();
    _descriptor_pool_create_info.pPoolSizes = _pool_sizes.data();
    _descriptor_pool_create_info.maxSets = 1;
    if (vkCreateDescriptorPool(SmoothieCore::getDevice(), &_descriptor_pool_create_info, nullptr, &m_DescriptorPool) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create descriptor pool.");
        return 1;
    }

    VkDescriptorSetAllocateInfo _descriptor_set_allocate_info{};
    _descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    _descriptor_set_allocate_info.pNext = nullptr;
    _descriptor_set_allocate_info.descriptorPool = m_DescriptorPool;
    _descriptor_set_allocate_info.descriptorSetCount = 1;
    _descriptor_set_allocate_info.pSetLayouts = &m_DescriptorSetLayout;

    if (vkAllocateDescriptorSets(SmoothieCore::getDevice(), &_descriptor_set_allocate_info, &m_DescriptorSet) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to allocate descriptor set.");
        return 1;
    }


    if (create_image_sky() != 0)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create sky image.");
        return 1;
    }

    VkDescriptorImageInfo _image_info{};
    _image_info.imageView = m_ImageView_Sky;
    _image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet _descriptor_write{};
    _descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    _descriptor_write.pNext = nullptr;
    _descriptor_write.dstSet = m_DescriptorSet;
    _descriptor_write.dstBinding = 0; //TODO: Fix this garbage
    _descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    _descriptor_write.descriptorCount = 1;
    _descriptor_write.pBufferInfo = nullptr;
    _descriptor_write.pImageInfo = &_image_info;

    vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_descriptor_write, 0, nullptr);


    return 0;
}

void Smoothie::DeferredRendering::Sky::destroy_descriptor_set()
{
    destroy_image_sky();

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

int Smoothie::DeferredRendering::Sky::create_pipeline_skybox()
{
    const auto& _draw_class = std::dynamic_pointer_cast<DeferredRendering::Drawing>(SmoothieCore::getDrawingClassPtr());
    if (_draw_class == nullptr)
    {
        SMOOTHIE_ERROR_WITH_SRC(R"(Failed to cast drawing class to "DeferredRendering::Drawing".)");
        return 1;
    }

    const VkDescriptorSetLayout _descriptor_set_layouts[] =
    {
        _draw_class->getGlobalDescriptorSetLayout(),
        m_DescriptorSetLayout
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
        SMOOTHIE_ERROR_WITH_SRC("Failed to create pipeline layout.");
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
        SMOOTHIE_ERROR_WITH_SRC("Failed to create pipeline!");
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
        SMOOTHIE_ERROR_WITH_SRC("Failed to create shader.");
        destroy();
        return 1;
    }

    if (create_descriptor_set() != 0)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create descriptor set.");
        destroy();
        return 1;
    }

    if (create_pipeline_skybox() != 0)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create skybox pipeline");
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
        drawData.descriptorSet,
        m_DescriptorSet
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
    destroy_descriptor_set();
    m_ShaderFile.destroy();

}
