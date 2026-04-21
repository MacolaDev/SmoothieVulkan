#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace Smoothie
{
	struct QueuedSubmitInfo
	{
		VkFence fence = nullptr;
		std::vector<VkSubmitInfo> submitInfos;
		VkQueue queue = nullptr;
	};

	class ImmediateCommandBuffer
	{
	public:
		enum class BufferState: int
		{
			Initial, Recording, Executable, Pending, Invalid
		};

		inline VkFence get_Fence() const {return m_Fence;}
		inline VkCommandPool get_CommandPool() const {return m_CommandPool;}
		inline VkCommandBuffer get_CommandBuffer() const {return m_CommandBuffer;}
		inline BufferState get_BufferState() const {return m_BufferState;}

		virtual int create();

		//Begin command buffer recording.
		virtual int begin();

		//End command buffer recording.
		virtual int end();

		//Submits command buffer to the GPU imediatelly.
		virtual void submit();

		//Multithreaded submissions
		virtual void submitAndWait();

		virtual void destroy();

		virtual ~ImmediateCommandBuffer() = default;

	protected:
		VkFence m_Fence = nullptr;
		VkCommandPool m_CommandPool = nullptr;
		VkCommandBuffer m_CommandBuffer = nullptr;
		BufferState m_BufferState = BufferState::Invalid;
	};



}