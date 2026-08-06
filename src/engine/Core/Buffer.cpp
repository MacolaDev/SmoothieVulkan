#include "Buffer.h"
#include "SmoothieCore.h"
#include "Multithreading.h"

int Smoothie::DefaultBuffer::create()
{
	VkBufferCreateInfo _buffer_create_info{};
	_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	_buffer_create_info.size = 1024;
	_buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VmaAllocationCreateInfo _allocation_create_info{};
	_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
	_allocation_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &_buffer_create_info, &_allocation_create_info, &m_Buffer, &m_BufferAllocation, nullptr) != VK_SUCCESS)
	{
	    SMOOTHIE_ERROR_WITH_SRC("Failed to create uniform buffer!");
		return 1;
	}

	ImmediateCommandBuffer _command_buffer;
	if (_command_buffer.create() != 0)
	{
		SMOOTHIE_ERROR_WITH_SRC("Failed to create immediate command buffer!");
		return 1;
	}
	_command_buffer.begin();
	vkCmdFillBuffer(_command_buffer.get_CommandBuffer(), m_Buffer, 0, VK_WHOLE_SIZE, 0);
	_command_buffer.end();
	_command_buffer.submitAndWait();
	_command_buffer.destroy();

	return 0;
}

void Smoothie::DefaultBuffer::destroy()
{
    if (m_Buffer != nullptr)
    {
        vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), m_Buffer, m_BufferAllocation);
    }
	m_Buffer = nullptr, m_BufferAllocation = nullptr;
}
