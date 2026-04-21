#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "vk_mem_alloc.h"

namespace Smoothie
{
	class Buffer_Base
	{
	public:

		virtual int create() = 0;
		virtual void resize_callback() = 0;
		virtual void destroy() = 0;

		inline VkBuffer getBuffer() const { return buffer; }
		inline VkBufferView getBufferView() const { return bufferView; }
		inline VmaAllocation getAllocation() const { return bufferAllocation; }
		inline VkDeviceSize getSize() const { return bufferSize; }

		virtual ~Buffer_Base() = default;
	protected:
		VkBuffer buffer = nullptr;
		VkBufferView bufferView = nullptr;
		VmaAllocation bufferAllocation = nullptr;
		VkDeviceSize bufferSize = 0;

	};
	
	//Default 1024 bytes size uniform/storage buffer initilized to 0.
	//It can be used as placeholder inside descriptor sets.
	class DefaultBuffer : public Buffer_Base
	{

	public:
		int create() override;
		void resize_callback() override;
		void destroy() override;
	};

}	