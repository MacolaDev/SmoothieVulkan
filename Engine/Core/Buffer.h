#pragma once
#include "Common.h"

namespace Smoothie
{
	class Buffer_Base
	{
	public:

		virtual int create() = 0;
		virtual void destroy() = 0;

		inline VkBuffer getBuffer() const { return m_Buffer; }
		inline VkBufferView getBufferView() const { return m_BufferView; }
		inline VmaAllocation getAllocation() const { return m_BufferAllocation; }
		inline VkDeviceSize getSize() const { return m_BufferSize; }

		virtual ~Buffer_Base() = default;
	protected:
		VkBuffer m_Buffer = nullptr;
		VkBufferView m_BufferView = nullptr;
		VmaAllocation m_BufferAllocation = nullptr;
		VkDeviceSize m_BufferSize = 0;

	};
	
	//Default 1024 bytes size uniform/storage buffer initialized to 0.
	//It can be used as placeholder inside descriptor sets.
	class DefaultBuffer : public Buffer_Base
	{

	public:
		int create() override;
		void destroy() override;
	};

}	