#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "Core/DescriptorType.h"
#include "vk_mem_alloc.h"

namespace Smoothie
{
	class BufferBase
	{
	public:

		virtual int create() = 0;
		virtual void resize_callback() = 0;
		virtual void destroy() = 0;

		inline VkBuffer getBuffer() const { return buffer; }
		inline VkBufferView getBufferView() const { return bufferView; }
		inline VmaAllocation getAllocation() const { return bufferAllocation; }
		inline VkDeviceSize getSize() const { return bufferSize; }
		BufferBase() = default;

	protected:
		VkBuffer buffer = nullptr;
		VkBufferView bufferView = nullptr;
		VmaAllocation bufferAllocation = nullptr;
		VkDeviceSize bufferSize = 0;

	};
	
	//Default 1024 bytes size uniform/storage buffer initilized to 0.
	//It can be used as placeholder inside descriptor sets.
	class DefaultBuffer : public BufferBase
	{

	public:
		int create() override;
		void resize_callback() override;
		void destroy() override;
	};

	//Buffer to be used in the descriptor set as uniform buffer
	class BufferUniform: public BufferBase
	{
	public:
		int create() override;
		void resize_callback() override;
		void destroy() override;
		
		BufferUniform() = default;

		std::vector<std::shared_ptr<BufferTypeBase>> buffer_data;
		std::string buffer_name;
	private: 
		std::vector<unsigned char> byte_data;
	};

}	