//
// Created by macola on 6/4/26.
//

#include "Cubemaps.h"

#include <array>

#include "Core/SmoothieCore.h"

static std::array<glm::mat4, 6> calculate_projectView_matrices()
{

    const glm::mat4 _projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    std::array<glm::mat4, 6> _matrices;

    _matrices[0] = _projection * glm::lookAt(glm::vec3({ 0.0f, 0.0f, 0.0f }), glm::vec3({ 1.0f, 0.0f, 0.0f }), glm::vec3({ 0.0f, -1.0f, 0.0f }));


    return _matrices;
}

static const std::array<glm::mat4, 6> g_projectView_Matrices = calculate_projectView_matrices();

int Smoothie::DeferredRendering::Helper_HDR_CubeMaps::create_images()
{
    VkImageCreateInfo _create_image_environment_map{};
    _create_image_environment_map.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    _create_image_environment_map.imageType = VK_IMAGE_TYPE_2D;
    _create_image_environment_map.extent = m_Extent_EnvironmentMap;
    _create_image_environment_map.mipLevels = 1;
    _create_image_environment_map.arrayLayers = 6;
    _create_image_environment_map.tiling = VK_IMAGE_TILING_OPTIMAL;
    _create_image_environment_map.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _create_image_environment_map.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    _create_image_environment_map.samples = VK_SAMPLE_COUNT_1_BIT;
    _create_image_environment_map.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    _create_image_environment_map.format = m_Format_EnvironmentMap;
    _create_image_environment_map.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VmaAllocationCreateInfo _allocation_create_info = {};
    _allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    _allocation_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    _allocation_create_info.priority = 1.0f;

    // VkFormatProperties _format_properties_environmentMap;
    // vkGetPhysicalDeviceFormatProperties(SmoothieCore::getPhysicalDevice(), _create_image_environment_map.format, &_format_properties_environmentMap);
    // assert((_format_properties_environmentMap.optimalTilingFeatures & _create_image_environment_map.usage) != true);


    if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_create_image_environment_map, &_allocation_create_info, &m_Image_EnvironmentMap, &m_Allocation_EnvironmentMap, nullptr) != VK_SUCCESS)
    {
        std::cout << "Failed to create environment image!" << std::endl;
        return 1;
    }

    VkImageViewCreateInfo _create_image_view_engivoment_map{};
    _create_image_view_engivoment_map.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    _create_image_view_engivoment_map.image = m_Image_EnvironmentMap;
    _create_image_view_engivoment_map.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    _create_image_view_engivoment_map.format = _create_image_environment_map.format;
    _create_image_view_engivoment_map.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _create_image_view_engivoment_map.subresourceRange.baseMipLevel = 0;
    _create_image_view_engivoment_map.subresourceRange.levelCount = 1;
    _create_image_view_engivoment_map.subresourceRange.baseArrayLayer = 0;
    _create_image_view_engivoment_map.subresourceRange.layerCount = 6;

    assert(_create_image_view_engivoment_map.image != nullptr);

    if (vkCreateImageView(SmoothieCore::getDevice(), &_create_image_view_engivoment_map, nullptr, &m_ImagView_EnvironmentMap) != VK_SUCCESS)
    {
        std::cout << "Failed to create environment image view!" << std::endl;
        return 1;
    }




    return 0;
}

void Smoothie::DeferredRendering::Helper_HDR_CubeMaps::destroy_images()
{
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
        std::cout << "Failed to create uniform buffer!" << std::endl;
        return 1;
    }

    if (vmaMapMemory(SmoothieCore::getVulkanMemoryAllocator(), m_Allocation_Uniform, &m_pBufferData) != VK_SUCCESS)
    {
        std::cout << "Failed to map uniform buffer!" << std::endl;
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
        vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), m_Buffer_Uniform, nullptr);
        m_Buffer_Uniform = nullptr;
    }
}

void Smoothie::DeferredRendering::Helper_HDR_CubeMaps::copy_buffer_data_uniform(VkCommandBuffer commandBuffer, const UniformBufferData& data)
{
    assert(m_pBufferData != nullptr);
    std::memcpy(m_pBufferData, &data, sizeof(UniformBufferData));

    VkBufferMemoryBarrier2 _buffer_barrier{};
    _buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    _buffer_barrier.pNext = nullptr;
    _buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    _buffer_barrier.srcAccessMask = VK_ACCESS_2_NONE;
    _buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _buffer_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
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
}

int Smoothie::DeferredRendering::Helper_HDR_CubeMaps::create_descriptor_set()
{
    if (create_images() != 0)
    {
        std::cerr << "Failed to descriptor images!" << std::endl;
        return 1;
    }

    if (create_buffer_uniform() != 0)
    {
        std::cerr << "Failed to create buffer uniform!" << std::endl;
        return 1;
    }

    std::vector<VkDescriptorSetLayoutBinding> _bindings;
    std::vector<VkDescriptorPoolSize> _pool_sizes;

    if (m_ShaderFile.get_DescriptorData(m_DescriptorSet_Name, _bindings, _pool_sizes) != 0)
    {
        std::cerr << "Failed to get descriptor data from shader!" << std::endl;
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
        std::cerr << "Failed to create descriptor layout!" << std::endl;
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
        std::cerr << "Failed to create descriptor pool!" << std::endl;
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
        std::cerr << "Failed to allocate descriptor set!" << std::endl;
        return 1;
    }

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
        std::cerr << "Failed to get drawing class!" << std::endl;
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
        std::cerr << "Failed to create pipeline layout!" << std::endl;
        return 1;
    }
    assert(m_PipelineLayout != nullptr);


    const VkShaderModule _shader_module = m_ShaderFile.get_ShaderModule();
    assert(_shader_module != nullptr);

    VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
    vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderPipelineCreateInfo.pName = "vertex_PBS_VERTEX";
    vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderPipelineCreateInfo.module = _shader_module;

    VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
    fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderPipelineCreateInfo.pName = "fragment_HDR_TO_CUBEMAP";
    fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderPipelineCreateInfo.module = _shader_module;


    return 0;
}

void Smoothie::DeferredRendering::Helper_HDR_CubeMaps::destroy_pipelines()
{



    if (m_PipelineLayout != nullptr)
    {
        vkDestroyPipelineLayout(SmoothieCore::getDevice(), m_PipelineLayout, nullptr);
        m_PipelineLayout = nullptr;
    }
}

int Smoothie::DeferredRendering::Helper_HDR_CubeMaps::create(const std::string &shader)
{
    if (m_ShaderFile.create(shader) != 0)
    {
        std::cerr << "Failed to create shader!" << std::endl;
        destroy();
        return 1;
    }

    if (create_descriptor_set() != 0)
    {
        std::cerr << "Failed to create descriptor set!" << std::endl;
        destroy();
        return 1;
    }

    if (create_pipelines() != 0)
    {
        std::cerr << "Failed to create pipelines!" << std::endl;
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
