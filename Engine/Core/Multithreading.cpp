#include "Multithreading.h"
#include "Core/SmoothieCore.h"
#include <iostream>
int Smoothie::ImmediateCommandBuffer::create()
{
	VkCommandPoolCreateInfo _poolCreateInfo{};
	_poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	_poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	_poolCreateInfo.queueFamilyIndex = SmoothieCore::getQueueFamilyGraphicsIndex();
	if (vkCreateCommandPool(SmoothieCore::getDevice(), &_poolCreateInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
	{
		std::cout << "Failed to create command pool" << std::endl;
		return 1;
	}


	VkCommandBufferAllocateInfo _bufferAllocateInfo{};
	_bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	_bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	_bufferAllocateInfo.commandPool = m_CommandPool;
	_bufferAllocateInfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(SmoothieCore::getDevice(), &_bufferAllocateInfo, &m_CommandBuffer) != VK_SUCCESS)
	{
		std::cout << "Failed to allocate command buffers" << std::endl;
		return 1;
	}

	m_BufferState = BufferState::Initial;

	VkFenceCreateInfo _fenceCreateInfo{};
	_fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	_fenceCreateInfo.flags = 0;
	if (vkCreateFence(SmoothieCore::getDevice(), &_fenceCreateInfo, nullptr, &m_Fence) != VK_SUCCESS)
	{
		std::cout << "Failed to create fence" << std::endl;
		return 1;
	}

	return 0;
}

int Smoothie::ImmediateCommandBuffer::begin()
{
	VkCommandBufferBeginInfo _beginInfo{};
	_beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	_beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(m_CommandBuffer, &_beginInfo) != VK_SUCCESS)
	{
		std::cout << "Failed to begin command buffer" << std::endl;
		return 1;
	}
	m_BufferState = BufferState::Recording;
	return 0;
}

int Smoothie::ImmediateCommandBuffer::end()
{

	if (vkEndCommandBuffer(m_CommandBuffer) != VK_SUCCESS)
	{
		std::cout << "Failed to end command buffer" << std::endl;
	}
	m_BufferState = BufferState::Executable;
	return 0;
}

void Smoothie::ImmediateCommandBuffer::submit()
{
	VkSubmitInfo _submitInfo{};
	_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	_submitInfo.commandBufferCount = 1;
	_submitInfo.pCommandBuffers = &m_CommandBuffer;

	const auto _queue = SmoothieCore::getGraphicsQueue();
	vkQueueWaitIdle(_queue);
	vkQueueSubmit(_queue, 1, &_submitInfo, m_Fence);
	vkWaitForFences(SmoothieCore::getDevice(), 1, &m_Fence, VK_TRUE, UINT64_MAX);
	vkResetFences(SmoothieCore::getDevice(), 1, &m_Fence);
}

void Smoothie::ImmediateCommandBuffer::submitAndWait()
{
	VkSubmitInfo _submitInfo{};
	_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	_submitInfo.commandBufferCount = 1;
	_submitInfo.pCommandBuffers = &m_CommandBuffer;

	QueuedSubmitInfo _enginedSubmitInfo{};
	_enginedSubmitInfo.fence = m_Fence;
	_enginedSubmitInfo.submitInfos = {_submitInfo};
	_enginedSubmitInfo.queue = SmoothieCore::getGraphicsQueue();

	SmoothieCore::SubmitToExecutionQueue(_enginedSubmitInfo);

	vkWaitForFences(SmoothieCore::getDevice(), 1, &m_Fence, VK_TRUE, UINT64_MAX);
	vkResetFences(SmoothieCore::getDevice(), 1, &m_Fence);
}

void Smoothie::ImmediateCommandBuffer::destroy()
{
	vkDestroyFence(SmoothieCore::getDevice(), m_Fence, nullptr), m_Fence = nullptr;
	vkDestroyCommandPool(SmoothieCore::getDevice(), m_CommandPool, nullptr), m_CommandPool = nullptr;
	m_BufferState = BufferState::Invalid;
	m_CommandBuffer = nullptr;
}
