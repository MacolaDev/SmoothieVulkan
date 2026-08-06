#pragma once
#include "../Common.h"

namespace Smoothie
{

	class ImmediateCommandBuffer
	{
	public:
		enum class BufferState: int
		{
			Initial, Recording, Executable, Pending, Invalid
		};

		inline VkCommandPool get_CommandPool() const {return m_CommandPool;}
		inline VkCommandBuffer get_CommandBuffer() const {return m_CommandBuffer;}
		inline BufferState get_BufferState() const {return m_BufferState;}

		virtual int create();

		//Begin command buffer recording.
		virtual int begin();

		//End command buffer recording.
		virtual int end();

		//Multithreaded submissions
		virtual void submitAndWait();

		virtual void destroy();

		virtual ~ImmediateCommandBuffer() = default;

	protected:
		VkCommandPool m_CommandPool = nullptr;
		VkCommandBuffer m_CommandBuffer = nullptr;
		BufferState m_BufferState = BufferState::Invalid;
	};



}