#include "Multithreading.h"
#define _SMOOTHIE_ENGINE
#include "Core/SmoothieCore.h"
#include <mutex>


ThreadFrendlyCommandData beginSingleTimeCommands()
{
	ThreadFrendlyCommandData data;


	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = SmoothieCore::getQueueFamilyGraphicsIndex();

	vkCreateCommandPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &data.pool);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = data.pool;
	allocInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(SmoothieCore::getDevice(), &allocInfo, &data.buffer);
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(data.buffer, &beginInfo);
	return data;

}

void endSingleTimeCommands(ThreadFrendlyCommandData& data)
{
	vkEndCommandBuffer(data.buffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &data.buffer;

	MultithreadSubmissions::addToGraphicsQueue(submitInfo);

	vkDestroyCommandPool(SmoothieCore::getDevice(), data.pool, nullptr);
	data.pool = nullptr;
	data.buffer = nullptr;


}

std::queue<VkSubmitInfo> MultithreadSubmissions::graphicsSubmitInfoQueue;
std::thread::id MultithreadSubmissions::renderingThreadID;
std::condition_variable MultithreadSubmissions::cv;

static std::mutex m;
static std::mutex waitMutex;
void MultithreadSubmissions::addToGraphicsQueue(const VkSubmitInfo& submitInfo)
{
	if (std::this_thread::get_id() == renderingThreadID)
	{
		vkQueueSubmit(SmoothieCore::getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(SmoothieCore::getGraphicsQueue());

		submitGraphicsQueue();
	}
	else
	{

		{
			std::lock_guard<std::mutex> lock(m);
			graphicsSubmitInfoQueue.push(submitInfo);
		}
		
		std::unique_lock<std::mutex> lock(waitMutex);
		cv.wait(lock, [] { return graphicsSubmitInfoQueue.empty(); });
	}
}

void MultithreadSubmissions::submitGraphicsQueue()
{
	if (graphicsSubmitInfoQueue.empty()) return;
	
	while (!graphicsSubmitInfoQueue.empty())
	{
		VkSubmitInfo& submitInfo = graphicsSubmitInfoQueue.front();
		vkQueueSubmit(SmoothieCore::getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(SmoothieCore::getGraphicsQueue());
		graphicsSubmitInfoQueue.pop();
	}
	cv.notify_all();
	
}

void MultithreadSubmissions::getRenderingThreadID()
{
	renderingThreadID = std::this_thread::get_id();
}
