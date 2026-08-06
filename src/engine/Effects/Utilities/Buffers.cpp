//
// Created by macola on 4/23/26.
//

#include "Buffers.h"
#include "Core/SmoothieCore.h"
#include "Core/Multithreading.h"

int Smoothie::DeferredRendering::Buffer_Uniform::create()
{
    VkBufferCreateInfo _buffer_info{};
    _buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    _buffer_info.size = m_BufferSize;
    assert(m_BufferSize == 0);
    _buffer_info.usage = m_Usage;

    VmaAllocationCreateInfo _allocation_info{};
    _allocation_info.usage = VMA_MEMORY_USAGE_AUTO;
    _allocation_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &_buffer_info, &_allocation_info, &m_Buffer, &m_BufferAllocation, nullptr) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create uniform buffer.");
        return 1;
    }

    ImmediateCommandBuffer _commandBuffer;
    if (_commandBuffer.create() != 0)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create immediate command buffer!");
        return 1;
    }
    _commandBuffer.begin();
    vkCmdFillBuffer(_commandBuffer.get_CommandBuffer(), m_Buffer, 0, VK_WHOLE_SIZE, 0);
    _commandBuffer.end();
    _commandBuffer.submitAndWait();
    _commandBuffer.destroy();

    return 0;
}

void Smoothie::DeferredRendering::Buffer_Uniform::destroy()
{
    if (m_Buffer != nullptr)
    {
        vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), m_Buffer, m_BufferAllocation);
    }
    m_Buffer = nullptr, m_BufferAllocation = nullptr;
    m_BufferSize = 0;
    m_Usage = 0;
}

int Smoothie::DeferredRendering::Buffer_MappedUniform::create()
{
    assert(m_BufferSize != 0);

    VkBufferCreateInfo _buffer_create_info{};
    _buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    _buffer_create_info.size = m_BufferSize;
    _buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VmaAllocationCreateInfo _buffer_allocation_info{};
    _buffer_allocation_info.usage = VMA_MEMORY_USAGE_AUTO;
    _buffer_allocation_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &_buffer_create_info, &_buffer_allocation_info, &m_Buffer, &m_BufferAllocation, nullptr) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create mapped buffer!");
        destroy();
        return 1;
    }

    m_Data.resize(m_BufferSize, 0);

    return 0;
}

void Smoothie::DeferredRendering::Buffer_MappedUniform::destroy()
{
    m_Data.clear();
    if (m_Buffer != nullptr)
    {
        vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), m_Buffer, m_BufferAllocation);
        m_Buffer = nullptr, m_BufferAllocation = nullptr;
    }

}

int Smoothie::DeferredRendering::Buffer_MappedUniform::map()
{
    if (m_pMappedData != nullptr) return 1;
    if (vmaMapMemory(SmoothieCore::getVulkanMemoryAllocator(), m_BufferAllocation, &m_pMappedData) != VK_SUCCESS){return 1;}
    return 0;
}

void Smoothie::DeferredRendering::Buffer_MappedUniform::unmap()
{
    if (m_pMappedData == nullptr) {return;}
    vmaUnmapMemory(SmoothieCore::getVulkanMemoryAllocator(), m_BufferAllocation);
    m_pMappedData = nullptr;
}

void Smoothie::DeferredRendering::Buffer_MappedUniform::copy_to_gpu(VkCommandBuffer transferBuffer) const
{

    if (m_Data.size() != m_BufferSize){return;}

    //std::memcpy(m_pMappedData, m_Data.data(), m_BufferSize);

    VkBufferMemoryBarrier2 _buffer_barrier{};
    _buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    _buffer_barrier.pNext = nullptr;
    _buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _buffer_barrier.srcAccessMask = VK_ACCESS_2_UNIFORM_READ_BIT;
    _buffer_barrier.dstStageMask =  VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    _buffer_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    _buffer_barrier.srcQueueFamilyIndex = 0;
    _buffer_barrier.dstQueueFamilyIndex = 0;
    _buffer_barrier.buffer = m_Buffer;
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
    vkCmdPipelineBarrier2(transferBuffer, &_dependencyInfo);

    vkCmdUpdateBuffer(transferBuffer, m_Buffer, 0, m_BufferSize, m_Data.data());

    _buffer_barrier.srcStageMask =  VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    _buffer_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    _buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    _buffer_barrier.dstAccessMask = VK_ACCESS_2_UNIFORM_READ_BIT;

    const VkBufferMemoryBarrier2 _buffer_barriers2[] = {_buffer_barrier};
    _dependencyInfo.bufferMemoryBarrierCount = std::size(_buffer_barriers2);
    _dependencyInfo.pBufferMemoryBarriers = _buffer_barriers2;

    vkCmdPipelineBarrier2(transferBuffer, &_dependencyInfo);
}
