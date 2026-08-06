//
// Created by macola on 6/4/26.
//

#include "Cubemaps.h"

#include "Core/SmoothieCore.h"
#include "Core/Multithreading.h"
#include "Effects/Deferred_Core.h"

#include "stb_image.h"

static std::array<glm::mat4, 6> calculate_projectView_matrices()
{

    const glm::mat4 _projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    std::array<glm::mat4, 6> _matrices{};

    _matrices[0] = _projection * glm::lookAt(glm::vec3({ 0.0f, 0.0f, 0.0f }), { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
    _matrices[1] = _projection * glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
    _matrices[3] = _projection * glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });
    _matrices[2] = _projection * glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    _matrices[4] = _projection * glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f });
    _matrices[5] = _projection * glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f });
    return _matrices;
}

static const std::array<glm::mat4, 6> g_projectView_Matrices = calculate_projectView_matrices();

int Smoothie::DeferredRendering::Helper_HDR_CubeMaps::create_images()
{
    VmaAllocationCreateInfo _allocation = {};
    _allocation.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    _allocation.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    _allocation.priority = 1.0f;

    //*********************************** Environment Map ***********************************//

    VkImageCreateInfo _image_environment_map{};
    _image_environment_map.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    _image_environment_map.imageType = VK_IMAGE_TYPE_2D;
    _image_environment_map.extent = {m_Size_EnvironmentMap, m_Size_EnvironmentMap, 1};
    _image_environment_map.mipLevels = 1;
    _image_environment_map.arrayLayers = 6;
    _image_environment_map.tiling = VK_IMAGE_TILING_OPTIMAL;
    _image_environment_map.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _image_environment_map.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    _image_environment_map.samples = VK_SAMPLE_COUNT_1_BIT;
    _image_environment_map.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    _image_environment_map.format = m_Format;
    _image_environment_map.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_image_environment_map, &_allocation, &m_Image_EnvironmentMap, &m_Allocation_EnvironmentMap, nullptr) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create environment image.");
        return 1;
    }

    assert(m_Image_EnvironmentMap != nullptr);

    VkImageViewCreateInfo _image_view_environment_map{};
    _image_view_environment_map.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    _image_view_environment_map.image = m_Image_EnvironmentMap;
    _image_view_environment_map.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    _image_view_environment_map.format = _image_environment_map.format;
    _image_view_environment_map.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _image_view_environment_map.subresourceRange.baseMipLevel = 0;
    _image_view_environment_map.subresourceRange.levelCount = 1;
    _image_view_environment_map.subresourceRange.baseArrayLayer = 0;
    _image_view_environment_map.subresourceRange.layerCount = 6;
    if (vkCreateImageView(SmoothieCore::getDevice(), &_image_view_environment_map, nullptr, &m_ImagView_EnvironmentMap) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create environment image view.");
        return 1;
    }

    //*********************************** Irradiance map ***********************************//

    VkImageCreateInfo _image_irradiance_map{};
    _image_irradiance_map.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    _image_irradiance_map.imageType = VK_IMAGE_TYPE_2D;
    _image_irradiance_map.extent = {m_Size_Irradiance, m_Size_Irradiance, 1};
    _image_irradiance_map.mipLevels = 1;
    _image_irradiance_map.arrayLayers = 6;
    _image_irradiance_map.tiling = VK_IMAGE_TILING_OPTIMAL;
    _image_irradiance_map.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _image_irradiance_map.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    _image_irradiance_map.samples = VK_SAMPLE_COUNT_1_BIT;
    _image_irradiance_map.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    _image_irradiance_map.format = m_Format;
    _image_irradiance_map.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_image_irradiance_map, &_allocation, &m_Image_Irradiance, &m_Allocation_Irradiance, nullptr) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create irradiance map.");
        return 1;
    }

    assert(m_Image_Irradiance != nullptr);

    VkImageViewCreateInfo _image_view_irradiance_map{};
    _image_view_irradiance_map.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    _image_view_irradiance_map.image = m_Image_Irradiance;
    _image_view_irradiance_map.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    _image_view_irradiance_map.format = m_Format;
    _image_view_irradiance_map.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _image_view_irradiance_map.subresourceRange.levelCount = 1;
    _image_view_irradiance_map.subresourceRange.layerCount = 6;
    if (vkCreateImageView(SmoothieCore::getDevice(), &_image_view_irradiance_map, nullptr, &m_ImagView_Irradiance) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create irradiance map render target image view.");
        return 1;
    }

    for (int i = 0; i < 6; i++)
    {
        VkImageViewCreateInfo _target_view_info{};
        _target_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        _target_view_info.image = m_Image_Irradiance;
        _target_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        _target_view_info.format = m_Format;
        _target_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        _target_view_info.subresourceRange.baseMipLevel = 0;
        _target_view_info.subresourceRange.levelCount = 1;
        _target_view_info.subresourceRange.baseArrayLayer = i;
        _target_view_info.subresourceRange.layerCount = 1;
        if (vkCreateImageView(SmoothieCore::getDevice(), &_target_view_info, nullptr, &m_Irradiance_Target_ImageViews[i]) != VK_SUCCESS)
        {
            SMOOTHIE_ERROR_WITH_SRC("Failed to create target image views for irradiance map.");
        }

    }

    //*********************************** Prefilter map ***********************************//

    VkImageCreateInfo _image_prefilter_map{};
    _image_prefilter_map.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    _image_prefilter_map.imageType = VK_IMAGE_TYPE_2D;
    _image_prefilter_map.extent.width = m_Size_Prefilter;
    _image_prefilter_map.extent.height = m_Size_Prefilter;
    _image_prefilter_map.extent.depth = 1;
    _image_prefilter_map.mipLevels = m_MipCount_Prefilter;
    _image_prefilter_map.arrayLayers = 6;
    _image_prefilter_map.tiling = VK_IMAGE_TILING_OPTIMAL;
    _image_prefilter_map.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _image_prefilter_map.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    _image_prefilter_map.samples = VK_SAMPLE_COUNT_1_BIT;
    _image_prefilter_map.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    _image_prefilter_map.format = m_Format;
    _image_prefilter_map.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_image_prefilter_map, &_allocation, &m_Image_Prefilter, &m_Allocation_Prefilter, nullptr) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create prefilter map image.");
        return 1;
    }

    VkImageViewCreateInfo _image_view_prefilter_map{};
    _image_view_prefilter_map.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    _image_view_prefilter_map.image = m_Image_Prefilter;
    _image_view_prefilter_map.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    _image_view_prefilter_map.format = m_Format;
    _image_view_prefilter_map.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _image_view_prefilter_map.subresourceRange.baseMipLevel = 0;
    _image_view_prefilter_map.subresourceRange.levelCount = m_MipCount_Prefilter;
    _image_view_prefilter_map.subresourceRange.baseArrayLayer = 0;
    _image_view_prefilter_map.subresourceRange.layerCount = 6;
    if (vkCreateImageView(SmoothieCore::getDevice(), &_image_view_prefilter_map, nullptr, &m_ImagView_Prefilter) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create prefilter map image view.");
        return 1;
    }


    for (int layer = 0; layer < 6; layer++)
    {
        auto& _image_view = m_Prefilter_Target_ImageViews[layer];
        _image_view.resize(m_MipCount_Prefilter);
        for (int mip = 0; mip < m_MipCount_Prefilter; mip++)
        {
            VkImageViewCreateInfo _target_view_info{};
            _target_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            _target_view_info.image = m_Image_Prefilter;
            _target_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            _target_view_info.format = m_Format;
            _target_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            _target_view_info.subresourceRange.baseMipLevel = mip;
            _target_view_info.subresourceRange.levelCount = 1;
            _target_view_info.subresourceRange.baseArrayLayer = layer;
            _target_view_info.subresourceRange.layerCount = 1;
            if (vkCreateImageView(SmoothieCore::getDevice(), &_target_view_info, nullptr, &_image_view[mip]) != VK_SUCCESS)
            {
                SMOOTHIE_ERROR_WITH_SRC("Failed to create target image views for prefilter map.");
            }
        }
    }


    return 0;
}

void Smoothie::DeferredRendering::Helper_HDR_CubeMaps::destroy_images()
{
    for (int layer = 0; layer < 6; layer++)
    {
        auto& _image_view = m_Prefilter_Target_ImageViews[layer];
        for (auto & mip : _image_view)
        {
            if (mip != nullptr)
            {
                vkDestroyImageView(SmoothieCore::getDevice(), mip, nullptr);
                mip = nullptr;
            }
        }
    }

    if (m_ImagView_Prefilter != nullptr)
    {
        vkDestroyImageView(SmoothieCore::getDevice(), m_ImagView_Prefilter, nullptr);
        m_ImagView_Prefilter = nullptr;
    }

    if (m_Image_Prefilter != nullptr)
    {
        vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_Image_Prefilter, m_Allocation_Prefilter);
        m_Image_Prefilter = nullptr; m_Allocation_Prefilter = nullptr;
    }

    for (auto& _view: m_Irradiance_Target_ImageViews)
    {
        if (_view != nullptr)
        {
            vkDestroyImageView(SmoothieCore::getDevice(), _view, nullptr);
            _view = nullptr;
        }
    }

    if (m_ImagView_Irradiance != nullptr)
    {
        vkDestroyImageView(SmoothieCore::getDevice(), m_ImagView_Irradiance, nullptr);
        m_ImagView_Irradiance = nullptr;
    }

    if (m_Image_Irradiance != nullptr)
    {
        vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_Image_Irradiance, m_Allocation_Irradiance);
        m_Image_Irradiance = nullptr, m_Allocation_Irradiance = nullptr;
    }

    if (m_ImagView_EnvironmentMap != nullptr)
    {
        vkDestroyImageView(SmoothieCore::getDevice(), m_ImagView_EnvironmentMap, nullptr);
        m_ImagView_EnvironmentMap = nullptr;
    }

    if (m_Image_EnvironmentMap != nullptr)
    {
        vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_Image_EnvironmentMap, m_Allocation_EnvironmentMap);
        m_Image_EnvironmentMap = nullptr, m_Allocation_EnvironmentMap = nullptr;
    }

}

int Smoothie::DeferredRendering::Helper_HDR_CubeMaps::create_buffer_uniform()
{
    VkBufferCreateInfo _buffer_create_info{};
    _buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    _buffer_create_info.size = sizeof(UniformBufferData);
    _buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VmaAllocationCreateInfo _buffer_allocation_info{};
    _buffer_allocation_info.usage = VMA_MEMORY_USAGE_AUTO;
    _buffer_allocation_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &_buffer_create_info, &_buffer_allocation_info, &m_Buffer_Uniform, &m_Allocation_Uniform, nullptr) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create uniform buffer.");
        return 1;
    }

    if (vmaMapMemory(SmoothieCore::getVulkanMemoryAllocator(), m_Allocation_Uniform, &m_pBufferData) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to map uniform buffer.");
        return 1;
    }

    return 0;
}

void Smoothie::DeferredRendering::Helper_HDR_CubeMaps::destroy_buffer_uniform()
{
    if (m_pBufferData != nullptr)
    {
        vmaUnmapMemory(SmoothieCore::getVulkanMemoryAllocator(), m_Allocation_Uniform);
        m_pBufferData = nullptr;
    }

    if (m_Buffer_Uniform != nullptr)
    {
        vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), m_Buffer_Uniform, m_Allocation_Uniform);
        m_Buffer_Uniform = nullptr;
    }
}

void Smoothie::DeferredRendering::Helper_HDR_CubeMaps::copy_buffer_data_uniform(VkCommandBuffer commandBuffer, const UniformBufferData& data)
{
    assert(m_pBufferData != nullptr);

    VkBufferMemoryBarrier2 _buffer_barrier{};
    _buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    _buffer_barrier.pNext = nullptr;
    _buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    _buffer_barrier.srcAccessMask = VK_ACCESS_2_UNIFORM_READ_BIT;
    _buffer_barrier.dstStageMask =  VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    _buffer_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    _buffer_barrier.srcQueueFamilyIndex = 0;
    _buffer_barrier.dstQueueFamilyIndex = 0;
    _buffer_barrier.buffer = m_Buffer_Uniform;
    _buffer_barrier.offset = 0;
    _buffer_barrier.size = VK_WHOLE_SIZE;

    const VkBufferMemoryBarrier2 _buffer_barriers[] = {_buffer_barrier};

    VkDependencyInfo _dependencyInfo{};
    _dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
    _dependencyInfo.pNext = nullptr;
    _dependencyInfo.dependencyFlags = 0;
    _dependencyInfo.memoryBarrierCount = 0;
    _dependencyInfo.pMemoryBarriers = nullptr;
    _dependencyInfo.bufferMemoryBarrierCount = std::size(_buffer_barriers);
    _dependencyInfo.pBufferMemoryBarriers = _buffer_barriers;
    _dependencyInfo.imageMemoryBarrierCount = 0;
    _dependencyInfo.pImageMemoryBarriers = nullptr;
    vkCmdPipelineBarrier2(commandBuffer, &_dependencyInfo);

    vkCmdUpdateBuffer(commandBuffer, m_Buffer_Uniform, 0, sizeof(UniformBufferData), &data);

    _buffer_barrier.srcStageMask =  VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    _buffer_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    _buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    _buffer_barrier.dstAccessMask = VK_ACCESS_2_UNIFORM_READ_BIT;

    const VkBufferMemoryBarrier2 _buffer_barriers2[] = {_buffer_barrier};
    _dependencyInfo.bufferMemoryBarrierCount = std::size(_buffer_barriers2);
    _dependencyInfo.pBufferMemoryBarriers = _buffer_barriers2;

    vkCmdPipelineBarrier2(commandBuffer, &_dependencyInfo);

}

int Smoothie::DeferredRendering::Helper_HDR_CubeMaps::create_descriptor_set()
{
    if (create_images() != 0)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to descriptor images.");
        return 1;
    }

    if (create_buffer_uniform() != 0)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create buffer uniform.");
        return 1;
    }

    std::vector<VkDescriptorSetLayoutBinding> _bindings;
    std::vector<VkDescriptorPoolSize> _pool_sizes;

    if (m_ShaderFile.get_DescriptorData(m_DescriptorSet_Name, _bindings, _pool_sizes) != 0)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to get descriptor data from shader.");
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

    VkDescriptorBufferInfo _buffer_info{};
    _buffer_info.buffer = m_Buffer_Uniform;
    _buffer_info.offset = 0;
    _buffer_info.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet _descriptor_write{};
    _descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    _descriptor_write.pNext = nullptr;
    _descriptor_write.dstSet = m_DescriptorSet;
    _descriptor_write.dstBinding = 2; //TODO: Fix this garbage
    _descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    _descriptor_write.descriptorCount = 1;
    _descriptor_write.pBufferInfo = &_buffer_info;

    vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_descriptor_write, 0, nullptr);

    return 0;
}

void Smoothie::DeferredRendering::Helper_HDR_CubeMaps::destroy_descriptor_set()
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

    destroy_buffer_uniform();
    destroy_images();

}

int Smoothie::DeferredRendering::Helper_HDR_CubeMaps::create_pipelines()
{
    const auto& _draw_class = std::dynamic_pointer_cast<DeferredRendering::Drawing>(SmoothieCore::getDrawingClassPtr());
    assert(_draw_class != nullptr);
    if (_draw_class == nullptr)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to get drawing class.");
        return 1;
    }

    const VkDescriptorSetLayout _descriptor_set_layouts[] = {_draw_class->getGlobalDescriptorSetLayout(), m_DescriptorSetLayout};

    assert(_draw_class->getGlobalDescriptorSetLayout() != nullptr);
    assert(m_DescriptorSetLayout != nullptr);

    VkPipelineLayoutCreateInfo _pipeline_layout_create_info{};
    _pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    _pipeline_layout_create_info.pNext = nullptr;
    _pipeline_layout_create_info.flags = 0;
    _pipeline_layout_create_info.setLayoutCount = std::size(_descriptor_set_layouts);
    _pipeline_layout_create_info.pSetLayouts = _descriptor_set_layouts;
    if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &_pipeline_layout_create_info, nullptr, &m_PipelineLayout) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create pipeline layout.");
        return 1;
    }
    assert(m_PipelineLayout != nullptr);


    VkShaderModule _shader_module = m_ShaderFile.get_ShaderModule();
    assert(_shader_module != nullptr);

    //*********************************** HDR to cubemap Pipeline ***********************************//

    VkPipelineShaderStageCreateInfo vertexShaderStage_cubemaps_vertex{};
    vertexShaderStage_cubemaps_vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStage_cubemaps_vertex.pName = m_StageEntryName_Vertex.c_str();
    vertexShaderStage_cubemaps_vertex.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStage_cubemaps_vertex.module = _shader_module;

    VkPipelineShaderStageCreateInfo fragmentShaderStage_hdr_to_cubemap{};
    fragmentShaderStage_hdr_to_cubemap.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStage_hdr_to_cubemap.pName = m_StageEntryName_HDR_To_Cubemap.c_str();
    fragmentShaderStage_hdr_to_cubemap.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStage_hdr_to_cubemap.module = _shader_module;


     const VkPipelineShaderStageCreateInfo _stages_hdr_to_cubemap[] = {vertexShaderStage_cubemaps_vertex, fragmentShaderStage_hdr_to_cubemap};

    VkPipelineRenderingCreateInfo _rendering_info{};
    _rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    _rendering_info.colorAttachmentCount = 1;
    _rendering_info.pColorAttachmentFormats = &m_Format;

    VkGraphicsPipelineCreateInfo _pipeline_hdr_to_cube_map{};
    _draw_class->lazy_populate_graphics_pipeline(_pipeline_hdr_to_cube_map);
    _pipeline_hdr_to_cube_map.pNext = &_rendering_info;
    _pipeline_hdr_to_cube_map.stageCount = std::size(_stages_hdr_to_cubemap);
    _pipeline_hdr_to_cube_map.pStages = _stages_hdr_to_cubemap;
    _pipeline_hdr_to_cube_map.layout = m_PipelineLayout;
    if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &_pipeline_hdr_to_cube_map, nullptr, &m_Pipeline_HDR_To_CubeMap) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create hdr_to_cubemap pipeline.");
        return 1;
    }

    //*********************************** Irradiance Pipeline ***********************************//

    VkPipelineShaderStageCreateInfo fragmentShaderStage_irradiance_map{};
    fragmentShaderStage_irradiance_map.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStage_irradiance_map.pName = m_StageEntryName_Irradiance_Map.c_str();
    fragmentShaderStage_irradiance_map.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStage_irradiance_map.module = _shader_module;

    const VkPipelineShaderStageCreateInfo _stages_irradiance[] = {vertexShaderStage_cubemaps_vertex, fragmentShaderStage_irradiance_map};

    VkGraphicsPipelineCreateInfo _pipeline_hdr_irradiance{};
    _draw_class->lazy_populate_graphics_pipeline(_pipeline_hdr_irradiance);
    _pipeline_hdr_irradiance.pNext = &_rendering_info;
    _pipeline_hdr_irradiance.stageCount = std::size(_stages_irradiance);
    _pipeline_hdr_irradiance.pStages = _stages_irradiance;
    _pipeline_hdr_irradiance.layout = m_PipelineLayout;

    if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &_pipeline_hdr_irradiance, nullptr, &m_Pipeline_Irradiance_Map) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create irradiance pipeline.");
        return 1;
    }


    //*********************************** Prefilter Pipeline ***********************************//

    VkPipelineShaderStageCreateInfo fragmentShaderStage_prefilter{};
    fragmentShaderStage_prefilter.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStage_prefilter.pName = m_StageEntryName_Prefilter_Map.c_str();
    fragmentShaderStage_prefilter.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStage_prefilter.module = _shader_module;

    const VkPipelineShaderStageCreateInfo _stages_prefilter[] = {vertexShaderStage_cubemaps_vertex, fragmentShaderStage_prefilter};

    VkGraphicsPipelineCreateInfo _pipeline_prefilter{};
    _draw_class->lazy_populate_graphics_pipeline(_pipeline_prefilter);
    _pipeline_prefilter.pNext = &_rendering_info;
    _pipeline_prefilter.stageCount = std::size(_stages_prefilter);
    _pipeline_prefilter.pStages = _stages_prefilter;
    _pipeline_prefilter.layout = m_PipelineLayout;

    if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &_pipeline_prefilter, nullptr, &m_Pipeline_Prefilter_Map) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create prefilter pipeline.");
        return 1;
    }

    return 0;
}

void Smoothie::DeferredRendering::Helper_HDR_CubeMaps::destroy_pipelines()
{
    if (m_Pipeline_Prefilter_Map != nullptr)
    {
        vkDestroyPipeline(SmoothieCore::getDevice(), m_Pipeline_Prefilter_Map, nullptr);
        m_Pipeline_Prefilter_Map = nullptr;
    }

    if (m_Pipeline_Irradiance_Map != nullptr)
    {
        vkDestroyPipeline(SmoothieCore::getDevice(), m_Pipeline_Irradiance_Map, nullptr);
        m_Pipeline_Irradiance_Map = nullptr;
    }

    if (m_Pipeline_HDR_To_CubeMap != nullptr)
    {
        vkDestroyPipeline(SmoothieCore::getDevice(), m_Pipeline_HDR_To_CubeMap, nullptr);
        m_Pipeline_HDR_To_CubeMap = nullptr;
    }

    if (m_PipelineLayout != nullptr)
    {
        vkDestroyPipelineLayout(SmoothieCore::getDevice(), m_PipelineLayout, nullptr);
        m_PipelineLayout = nullptr;
    }

}

int Smoothie::DeferredRendering::Helper_HDR_CubeMaps::create_cubmap_from_hdr(const std::string &filepath)
{
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float* data = stbi_loadf(filepath.c_str(), &width, &height, &nrComponents, 4);
    if (data == nullptr)
    {
        SmoothieCore::logError_Formated("Can't load HDR image: {}", filepath);
        return 1;
    }


    VkImage _hdr_image = nullptr;
    VmaAllocation _hdr_image_allocation = nullptr;

    VkImageCreateInfo _hdr_image_create_info{};
    _hdr_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    _hdr_image_create_info.imageType = VK_IMAGE_TYPE_2D;
    _hdr_image_create_info.extent.width = width;
    _hdr_image_create_info.extent.height = height;
    _hdr_image_create_info.extent.depth = 1;
    _hdr_image_create_info.mipLevels = 1;
    _hdr_image_create_info.arrayLayers = 1;
    _hdr_image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    _hdr_image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _hdr_image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    _hdr_image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    _hdr_image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    _hdr_image_create_info.format = VK_FORMAT_R32G32B32A32_SFLOAT;

    VmaAllocationCreateInfo vmaImageAllocationInfo{};
    vmaImageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
    if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_hdr_image_create_info, &vmaImageAllocationInfo, &_hdr_image, &_hdr_image_allocation, nullptr) != VK_SUCCESS)
    {
        SmoothieCore::logError_Formated("{} Failed to create image.", SMOOTHIE_LOCATION);
        return 1;
    }


    VkImageView _hdr_image_view = nullptr;

    VkImageViewCreateInfo _hdr_image_view_create_info{};
    _hdr_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    _hdr_image_view_create_info.image = _hdr_image;
    _hdr_image_view_create_info.format = _hdr_image_create_info.format;
    _hdr_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    _hdr_image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _hdr_image_view_create_info.subresourceRange.baseMipLevel = 0;
    _hdr_image_view_create_info.subresourceRange.levelCount = 1;
    _hdr_image_view_create_info.subresourceRange.baseArrayLayer = 0;
    _hdr_image_view_create_info.subresourceRange.layerCount = 1;
    if (vkCreateImageView(SmoothieCore::getDevice(), &_hdr_image_view_create_info, nullptr, &_hdr_image_view) != VK_SUCCESS)
    {
        SmoothieCore::logError_Formated("{} Failed to create image view.", SMOOTHIE_LOCATION);
        return 1;
    }



    VkDescriptorImageInfo _image_write_info{};
    _image_write_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    _image_write_info.imageView = _hdr_image_view;

    VkWriteDescriptorSet _write_descriptor_set{};
    _write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    _write_descriptor_set.pNext = nullptr;
    _write_descriptor_set.dstSet = m_DescriptorSet;
    _write_descriptor_set.dstBinding = 1; //TODO: Fix this garbage
    _write_descriptor_set.dstArrayElement = 0;
    _write_descriptor_set.descriptorCount = 1; //TODO: Fix this garbage
    _write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    _write_descriptor_set.pImageInfo = &_image_write_info;
    vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_write_descriptor_set, 0, nullptr);


    VkBuffer _staging_buffer = nullptr;
    VmaAllocation _staging_buffer_allocation = nullptr;
    VmaAllocationCreateInfo _staging_buffer_allocation_info = {};
    _staging_buffer_allocation_info.usage = VMA_MEMORY_USAGE_AUTO;
    _staging_buffer_allocation_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    _staging_buffer_allocation_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkMemoryRequirements _memoryRequirements = {};
    vkGetImageMemoryRequirements(SmoothieCore::getDevice(), _hdr_image, &_memoryRequirements);

    VkBufferCreateInfo stagingBufferCreateInfo{};
    stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferCreateInfo.size = _memoryRequirements.size;
    stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &stagingBufferCreateInfo, &_staging_buffer_allocation_info, &_staging_buffer, &_staging_buffer_allocation, nullptr) != VK_SUCCESS)
    {
        SmoothieCore::logError_Formated("{} Failed to create staging buffer.", SMOOTHIE_LOCATION);
        return 1;
    }

    if (vmaCopyMemoryToAllocation(SmoothieCore::getVulkanMemoryAllocator(), data, _staging_buffer_allocation, 0, stagingBufferCreateInfo.size) != VK_SUCCESS)
    {
        SmoothieCore::logError_Formated("{} Failed to copy memory to allocation.", SMOOTHIE_LOCATION);
        return 1;
    }

    stbi_image_free(data);
    data = nullptr;


    std::array<VkImageView, 6> _target_views{};
    for (int i = 0; i < 6; ++i)
    {
        VkImageViewCreateInfo _target_view_info{};
        _target_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        _target_view_info.image = m_Image_EnvironmentMap;
        _target_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        _target_view_info.format = m_Format;
        _target_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        _target_view_info.subresourceRange.baseMipLevel = 0;
        _target_view_info.subresourceRange.levelCount = 1;
        _target_view_info.subresourceRange.baseArrayLayer = i;
        _target_view_info.subresourceRange.layerCount = 1;
        if (vkCreateImageView(SmoothieCore::getDevice(), &_target_view_info, nullptr, &_target_views[i]) != VK_SUCCESS)
        {
            SmoothieCore::logError_Formated("{} Failed to create image view.", SMOOTHIE_LOCATION);
            return 1;
        }
    }

    const auto& _draw_class = std::dynamic_pointer_cast<Smoothie::DeferredRendering::Drawing>(SmoothieCore::getDrawingClassPtr());
    if (_draw_class == nullptr)
    {
        assert(false);
        SmoothieCore::logError_Formated("{} Failed to get drawing class.", SMOOTHIE_LOCATION);
        return 1;
    }

    ImmediateCommandBuffer _immediateCommandBuffer;
    _immediateCommandBuffer.create();
    _immediateCommandBuffer.begin();
    auto _commandBuffer = _immediateCommandBuffer.get_CommandBuffer();

    VkImageMemoryBarrier2 _barrier_hdr_image{};
    _barrier_hdr_image.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _barrier_hdr_image.pNext = nullptr;
    _barrier_hdr_image.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    _barrier_hdr_image.srcAccessMask = VK_ACCESS_2_NONE;
    _barrier_hdr_image.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    _barrier_hdr_image.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    _barrier_hdr_image.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _barrier_hdr_image.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    _barrier_hdr_image.image = _hdr_image;
    _barrier_hdr_image.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _barrier_hdr_image.subresourceRange.baseMipLevel = 0;
    _barrier_hdr_image.subresourceRange.levelCount = 1;
    _barrier_hdr_image.subresourceRange.baseArrayLayer = 0;
    _barrier_hdr_image.subresourceRange.layerCount = 1;

    VkDependencyInfo _dependencyInfo{};
    _dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    _dependencyInfo.pNext = nullptr;
    _dependencyInfo.imageMemoryBarrierCount = 1;
    _dependencyInfo.pImageMemoryBarriers = &_barrier_hdr_image;
    vkCmdPipelineBarrier2(_commandBuffer, &_dependencyInfo);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { static_cast<unsigned int>(width), static_cast<unsigned int>(height), 1};
    vkCmdCopyBufferToImage(_commandBuffer, _staging_buffer, _hdr_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    _barrier_hdr_image.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    _barrier_hdr_image.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    _barrier_hdr_image.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    _barrier_hdr_image.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    _barrier_hdr_image.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    _barrier_hdr_image.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkImageMemoryBarrier2 _barrier_environment_map = _barrier_hdr_image;
    _barrier_environment_map.image = m_Image_EnvironmentMap;
    _barrier_environment_map.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    _barrier_environment_map.srcAccessMask = VK_ACCESS_2_NONE;
    _barrier_environment_map.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    _barrier_environment_map.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    _barrier_environment_map.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _barrier_environment_map.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _barrier_environment_map.subresourceRange.layerCount = 6;

    const VkImageMemoryBarrier2 _memory_barriers[] = {_barrier_hdr_image, _barrier_environment_map};

    _dependencyInfo.imageMemoryBarrierCount = std::size(_memory_barriers);
    _dependencyInfo.pImageMemoryBarriers = _memory_barriers;
    vkCmdPipelineBarrier2(_commandBuffer, &_dependencyInfo);

    vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline_HDR_To_CubeMap);

    for (int i = 0; i < 6; i++)
    {
        copy_buffer_data_uniform(_commandBuffer, {g_projectView_Matrices[i], 0.0f});

        VkRenderingAttachmentInfo _attachment_info{};
        _attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        _attachment_info.imageView = _target_views[i];
        _attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        _attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        _attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        _attachment_info.clearValue.color.float32[0] = 1.0f;
        _attachment_info.clearValue.color.float32[1] = 1.0f;
        _attachment_info.clearValue.color.float32[2] = 1.0f;
        _attachment_info.clearValue.color.float32[3] = 1.0f;

        VkRenderingInfo _rendering_info{};
        _rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        _rendering_info.renderArea.extent.width = m_Size_EnvironmentMap;
        _rendering_info.renderArea.extent.height = m_Size_EnvironmentMap;
        _rendering_info.renderArea.offset.x = 0;
        _rendering_info.renderArea.offset.y = 0;
        _rendering_info.layerCount = 1;
        _rendering_info.colorAttachmentCount = 1;
        _rendering_info.pColorAttachments = &_attachment_info;
        vkCmdBeginRendering(_commandBuffer, &_rendering_info);

        VkViewport _viewport{};
        _viewport.x = 0.0f;
        _viewport.y = 0.0f;
        _viewport.width = static_cast<float>(m_Size_EnvironmentMap);
        _viewport.height = static_cast<float>(m_Size_EnvironmentMap);
        _viewport.minDepth = 0.0f;
        _viewport.maxDepth = 1.0f;
        vkCmdSetViewport(_commandBuffer, 0, 1, &_viewport);

        VkRect2D _scissor{};
        _scissor.offset = { 0, 0 };
        _scissor.extent.height = m_Size_EnvironmentMap;
        _scissor.extent.width = m_Size_EnvironmentMap;
        vkCmdSetScissor(_commandBuffer, 0, 1, &_scissor);

        const VkDescriptorSet _descriptor_sets[] = {_draw_class->getGlobalDescriptorSet(), m_DescriptorSet};

        vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, std::size(_descriptor_sets) ,_descriptor_sets, 0, nullptr);
        vkCmdDraw(_commandBuffer, 36, 1, 0, 0);

        vkCmdEndRendering(_commandBuffer);
    }

    _barrier_environment_map.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    _barrier_environment_map.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    _barrier_environment_map.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _barrier_environment_map.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    _barrier_environment_map.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _barrier_environment_map.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    _dependencyInfo.imageMemoryBarrierCount = 1;
    _dependencyInfo.pImageMemoryBarriers = &_barrier_environment_map;
    vkCmdPipelineBarrier2(_commandBuffer, &_dependencyInfo);

    _immediateCommandBuffer.end();
    _immediateCommandBuffer.submitAndWait();
    _immediateCommandBuffer.destroy();

    for (int i = 0; i < 6; ++i)
    {
        if (_target_views[i] != nullptr) {vkDestroyImageView(SmoothieCore::getDevice(), _target_views[i], nullptr); _target_views[i] = nullptr;}
    }

    vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), _staging_buffer, _staging_buffer_allocation);
    vkDestroyImageView(SmoothieCore::getDevice(), _hdr_image_view, nullptr);
    vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), _hdr_image, _hdr_image_allocation);


    return 0;
}

int Smoothie::DeferredRendering::Helper_HDR_CubeMaps::copy_env_map_to_skybox_cubemap(VkImage skybox_image, VkExtent3D sky_extend, VkFormat sky_format)
{

    assert(skybox_image != nullptr);

    ImmediateCommandBuffer _immediateCommandBuffer;
    _immediateCommandBuffer.create();
    _immediateCommandBuffer.begin();

    auto _command_buffer = _immediateCommandBuffer.get_CommandBuffer();

    VkImageMemoryBarrier2 _barrier_src_image{};
    _barrier_src_image.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _barrier_src_image.pNext = nullptr;
    _barrier_src_image.srcStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _barrier_src_image.srcAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    _barrier_src_image.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    _barrier_src_image.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    _barrier_src_image.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    _barrier_src_image.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    _barrier_src_image.image = m_Image_EnvironmentMap;
    _barrier_src_image.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _barrier_src_image.subresourceRange.baseMipLevel = 0;
    _barrier_src_image.subresourceRange.levelCount = 1;
    _barrier_src_image.subresourceRange.baseArrayLayer = 0;
    _barrier_src_image.subresourceRange.layerCount = 6;


    VkImageMemoryBarrier2 _barrier_dst_image = _barrier_src_image;
    _barrier_dst_image.image = skybox_image;
    _barrier_dst_image.srcStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _barrier_dst_image.srcAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    _barrier_dst_image.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    _barrier_dst_image.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    _barrier_dst_image.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    _barrier_dst_image.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    const VkImageMemoryBarrier2 _barriers[] = {_barrier_src_image, _barrier_dst_image};

    VkDependencyInfo _dependencyInfo{};
    _dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    _dependencyInfo.pNext = nullptr;
    _dependencyInfo.imageMemoryBarrierCount = std::size(_barriers);
    _dependencyInfo.pImageMemoryBarriers = _barriers;
    vkCmdPipelineBarrier2(_command_buffer, &_dependencyInfo);

    VkImageCopy _copy{};
    _copy.srcSubresource.layerCount = 6;
    _copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _copy.srcSubresource.mipLevel = 0;
    _copy.dstSubresource = _copy.srcSubresource;
    _copy.srcOffset = _copy.dstOffset = {0, 0, 0};
    _copy.extent = sky_extend;
    vkCmdCopyImage(_command_buffer, m_Image_EnvironmentMap, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, skybox_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &_copy);


    _barrier_src_image.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    _barrier_src_image.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    _barrier_src_image.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _barrier_src_image.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    _barrier_src_image.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    _barrier_src_image.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


    _barrier_dst_image.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    _barrier_dst_image.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    _barrier_dst_image.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _barrier_dst_image.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    _barrier_dst_image.oldLayout =  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    _barrier_dst_image.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    const VkImageMemoryBarrier2 _barriers2[] = {_barrier_src_image, _barrier_dst_image};

    _dependencyInfo.imageMemoryBarrierCount = std::size(_barriers2);
    _dependencyInfo.pImageMemoryBarriers = _barriers2;
    vkCmdPipelineBarrier2(_command_buffer, &_dependencyInfo);

    _immediateCommandBuffer.end();
    _immediateCommandBuffer.submitAndWait();
    _immediateCommandBuffer.destroy();


    return 0;
}

int Smoothie::DeferredRendering::Helper_HDR_CubeMaps::calculate_irradiance_prefilter_maps()
{

    assert(m_Image_EnvironmentMap != nullptr);
    assert(m_Image_Irradiance != nullptr);
    assert(m_Pipeline_Irradiance_Map != nullptr);
    for (auto& _view: m_Irradiance_Target_ImageViews)
    {
        assert(_view != nullptr);
    }

    const auto& _draw_class = std::dynamic_pointer_cast<DeferredRendering::Drawing>(SmoothieCore::getDrawingClassPtr());
    assert(_draw_class != nullptr);
    VkDescriptorSet _drawing_class_descriptor_set = _draw_class->getGlobalDescriptorSet();
    assert(_drawing_class_descriptor_set != nullptr);
    assert(m_Pipeline_Prefilter_Map != nullptr);
    for (auto& _view: m_Prefilter_Target_ImageViews)
    {
        assert(_view.empty() == 0);
        for (auto& _image: _view)
        {
            assert(_image != nullptr);
        }
    }


    //Update descriptor set with environment map
    VkDescriptorImageInfo _image_write_info{};
    _image_write_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    _image_write_info.imageView = m_ImagView_EnvironmentMap;

    VkWriteDescriptorSet _write_descriptor_set{};
    _write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    _write_descriptor_set.pNext = nullptr;
    _write_descriptor_set.dstSet = m_DescriptorSet;
    _write_descriptor_set.dstBinding = 0; //TODO: Fix this garbage
    _write_descriptor_set.dstArrayElement = 0;
    _write_descriptor_set.descriptorCount = 1;
    _write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    _write_descriptor_set.pImageInfo = &_image_write_info;
    vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_write_descriptor_set, 0, nullptr);


    ImmediateCommandBuffer _immediateCommandBuffer;
    _immediateCommandBuffer.create();
    _immediateCommandBuffer.begin();
    auto _commandBuffer = _immediateCommandBuffer.get_CommandBuffer();

    VkImageMemoryBarrier2 _barrier_irradiance{};
    _barrier_irradiance.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    _barrier_irradiance.pNext = nullptr;
    _barrier_irradiance.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    _barrier_irradiance.srcAccessMask = VK_ACCESS_2_NONE;
    _barrier_irradiance.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    _barrier_irradiance.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    _barrier_irradiance.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _barrier_irradiance.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _barrier_irradiance.image = m_Image_Irradiance;
    _barrier_irradiance.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _barrier_irradiance.subresourceRange.baseMipLevel = 0;
    _barrier_irradiance.subresourceRange.levelCount = 1;
    _barrier_irradiance.subresourceRange.baseArrayLayer = 0;
    _barrier_irradiance.subresourceRange.layerCount = 6;

    VkImageMemoryBarrier2 _barrier_prefilter = _barrier_irradiance;
    _barrier_prefilter.image = m_Image_Prefilter;
    _barrier_prefilter.subresourceRange.levelCount = m_MipCount_Prefilter;

    const VkImageMemoryBarrier2 _barriers[] = {_barrier_irradiance, _barrier_prefilter};

    VkDependencyInfo _dependencyInfo{};
    _dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    _dependencyInfo.pNext = nullptr;
    _dependencyInfo.imageMemoryBarrierCount = std::size(_barriers);
    _dependencyInfo.pImageMemoryBarriers = _barriers;
    vkCmdPipelineBarrier2(_commandBuffer, &_dependencyInfo);


    // Calculate irradiance map
    vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline_Irradiance_Map);

    for (int i = 0; i < 6; i++)
    {
        copy_buffer_data_uniform(_commandBuffer, {g_projectView_Matrices[i], 0.0f});

        VkRenderingAttachmentInfo _attachment_info{};
        _attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        _attachment_info.imageView = m_Irradiance_Target_ImageViews[i];
        _attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        _attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        _attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        _attachment_info.clearValue.color.float32[0] = 1.0f;
        _attachment_info.clearValue.color.float32[1] = 1.0f;
        _attachment_info.clearValue.color.float32[2] = 1.0f;
        _attachment_info.clearValue.color.float32[3] = 1.0f;

        VkRenderingInfo _rendering_info{};
        _rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        _rendering_info.renderArea.extent.width = m_Size_Irradiance;
        _rendering_info.renderArea.extent.height = m_Size_Irradiance;
        _rendering_info.renderArea.offset.x = 0;
        _rendering_info.renderArea.offset.y = 0;
        _rendering_info.layerCount = 1;
        _rendering_info.colorAttachmentCount = 1;
        _rendering_info.pColorAttachments = &_attachment_info;
        vkCmdBeginRendering(_commandBuffer, &_rendering_info);

        VkViewport _viewport{};
        _viewport.x = 0.0f;
        _viewport.y = 0.0f;
        _viewport.width = static_cast<float>(m_Size_Irradiance);
        _viewport.height = static_cast<float>(m_Size_Irradiance);
        _viewport.minDepth = 0.0f;
        _viewport.maxDepth = 1.0f;
        vkCmdSetViewport(_commandBuffer, 0, 1, &_viewport);

        VkRect2D _scissor{};
        _scissor.offset = { 0, 0 };
        _scissor.extent.height = m_Size_Irradiance;
        _scissor.extent.width = m_Size_Irradiance;
        vkCmdSetScissor(_commandBuffer, 0, 1, &_scissor);

        const VkDescriptorSet _descriptor_sets[] = {_drawing_class_descriptor_set, m_DescriptorSet};

        vkCmdBindDescriptorSets(
            _commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout, 0,
            std::size(_descriptor_sets) ,
            _descriptor_sets, 0, nullptr);
        vkCmdDraw(_commandBuffer, 36, 1, 0, 0);
        vkCmdEndRendering(_commandBuffer);
    }


    // _immediateCommandBuffer.end();
    // _immediateCommandBuffer.submitAndWait();
    // _immediateCommandBuffer.begin();

    // Calculate prefilter map
    vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline_Prefilter_Map);
    for (int layer = 0; layer < 6; layer++)
    {
        auto& _image_view = m_Prefilter_Target_ImageViews[layer];
        for (int mip = 0; mip < m_MipCount_Prefilter; mip++)
        {
            auto _size = static_cast<unsigned int>(m_Size_Prefilter * std::pow(0.5, mip));
            const float _roughness = static_cast<float>(mip) / static_cast<float>(m_MipCount_Prefilter - 1);
            copy_buffer_data_uniform(_commandBuffer, {g_projectView_Matrices[layer], _roughness});

            VkRenderingAttachmentInfo _attachment_info{};
            _attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            _attachment_info.imageView = _image_view[mip];
            _attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            _attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            _attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            _attachment_info.clearValue.color.float32[0] = 1.0f;
            _attachment_info.clearValue.color.float32[1] = 1.0f;
            _attachment_info.clearValue.color.float32[2] = 1.0f;
            _attachment_info.clearValue.color.float32[3] = 1.0f;

            VkRenderingInfo _rendering_info{};
            _rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
            _rendering_info.renderArea.extent.width = _size;
            _rendering_info.renderArea.extent.height = _size;
            _rendering_info.renderArea.offset.x = 0;
            _rendering_info.renderArea.offset.y = 0;
            _rendering_info.layerCount = 1;
            _rendering_info.colorAttachmentCount = 1;
            _rendering_info.pColorAttachments = &_attachment_info;
            vkCmdBeginRendering(_commandBuffer, &_rendering_info);

            VkViewport _viewport{};
            _viewport.x = 0.0f;
            _viewport.y = 0.0f;
            _viewport.width = static_cast<float>(_size);
            _viewport.height = static_cast<float>(_size);
            _viewport.minDepth = 0.0f;
            _viewport.maxDepth = 1.0f;
            vkCmdSetViewport(_commandBuffer, 0, 1, &_viewport);

            VkRect2D _scissor{};
            _scissor.offset = { 0, 0 };
            _scissor.extent.height = _size;
            _scissor.extent.width = _size;
            vkCmdSetScissor(_commandBuffer, 0, 1, &_scissor);

            const VkDescriptorSet _descriptor_sets[] = {_drawing_class_descriptor_set, m_DescriptorSet};

            vkCmdBindDescriptorSets(
                _commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_PipelineLayout, 0,
                std::size(_descriptor_sets) ,
                _descriptor_sets, 0, nullptr);

            vkCmdDraw(_commandBuffer, 36, 1, 0, 0);
            vkCmdEndRendering(_commandBuffer);
        }
    }

    _barrier_irradiance.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    _barrier_irradiance.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    _barrier_irradiance.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    _barrier_irradiance.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    _barrier_irradiance.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _barrier_irradiance.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    _barrier_prefilter = _barrier_irradiance;
    _barrier_prefilter.image = m_Image_Prefilter;
    _barrier_prefilter.subresourceRange.levelCount = m_MipCount_Prefilter;

    const VkImageMemoryBarrier2 _barriers2[] = {_barrier_irradiance, _barrier_prefilter};

    _dependencyInfo.imageMemoryBarrierCount = std::size(_barriers2);
    _dependencyInfo.pImageMemoryBarriers = _barriers2;
    vkCmdPipelineBarrier2(_commandBuffer, &_dependencyInfo);


    _immediateCommandBuffer.end();
    _immediateCommandBuffer.submitAndWait();
    _immediateCommandBuffer.destroy();


    return 0;
}

int Smoothie::DeferredRendering::Helper_HDR_CubeMaps::create(const std::string &shader)
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

    if (create_pipelines() != 0)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create pipelines.");
        destroy();
        return 1;
    }

    return 0;
}

void Smoothie::DeferredRendering::Helper_HDR_CubeMaps::destroy()
{
    destroy_pipelines();
    destroy_descriptor_set();
    m_ShaderFile.destroy();
}
