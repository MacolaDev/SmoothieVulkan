#include "Multithreading.h"
#include "SmoothieCore.h"

int Smoothie::ImmediateCommandBuffer::create()
{
	VkCommandPoolCreateInfo _pool_create_info{};
	_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	_pool_create_info.queueFamilyIndex = SmoothieCore::getQueueFamilyGraphicsIndex();
	if (vkCreateCommandPool(SmoothieCore::getDevice(), &_pool_create_info, nullptr, &m_CommandPool) != VK_SUCCESS)
	{
		SMOOTHIE_ERROR_WITH_SRC("Failed to create command pool");
		return 1;
	}

	VkCommandBufferAllocateInfo _bufferAllocateInfo{};
	_bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	_bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	_bufferAllocateInfo.commandPool = m_CommandPool;
	_bufferAllocateInfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(SmoothieCore::getDevice(), &_bufferAllocateInfo, &m_CommandBuffer) != VK_SUCCESS)
	{
	    SMOOTHIE_ERROR_WITH_SRC("Failed to allocate command buffer!");
		return 1;
	}

	m_BufferState = BufferState::Initial;

	return 0;
}

int Smoothie::ImmediateCommandBuffer::begin()
{
	VkCommandBufferBeginInfo _beginInfo{};
	_beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	_beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(m_CommandBuffer, &_beginInfo) != VK_SUCCESS)
	{
		SMOOTHIE_ERROR_WITH_SRC("Failed to begin command buffer!");
		return 1;
	}
	m_BufferState = BufferState::Recording;
	return 0;
}

int Smoothie::ImmediateCommandBuffer::end()
{

	if (vkEndCommandBuffer(m_CommandBuffer) != VK_SUCCESS)
	{
		SMOOTHIE_ERROR_WITH_SRC("Failed to end command buffer!");
	}
	m_BufferState = BufferState::Executable;
	return 0;
}

void Smoothie::ImmediateCommandBuffer::submitAndWait()
{
	VkSubmitInfo _submitInfo{};
	_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	_submitInfo.commandBufferCount = 1;
	_submitInfo.pCommandBuffers = &m_CommandBuffer;
	{
	    auto _queue = SmoothieCore::getGraphicsQueue();
	    std::lock_guard<std::mutex> _lock(SmoothieCore::getQueueSubmitMutex());
	    vkQueueWaitIdle(_queue);
	    vkQueueSubmit(_queue, 1, &_submitInfo, nullptr);
	    vkQueueWaitIdle(_queue);
	}


}

void Smoothie::ImmediateCommandBuffer::destroy()
{
    if (m_CommandBuffer != nullptr)
    {
        vkDestroyCommandPool(SmoothieCore::getDevice(), m_CommandPool, nullptr), m_CommandPool = nullptr;
    }
	m_BufferState = BufferState::Invalid;
	m_CommandBuffer = nullptr;
}
