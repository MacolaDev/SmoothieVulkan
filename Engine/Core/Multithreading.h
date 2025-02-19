#pragma once
#include <mutex>
#include <queue>
#include <vulkan/vulkan.h>
#include <thread>
#include <queue>
#include <condition_variable>

struct ThreadFrendlyCommandData
{
	VkCommandBuffer buffer = nullptr;
	VkCommandPool pool = nullptr;
};

void endSingleTimeCommands(ThreadFrendlyCommandData& commandBuffer);
ThreadFrendlyCommandData beginSingleTimeCommands();


class MultithreadSubmissions 
{
public:

	static void addToGraphicsQueue(const VkSubmitInfo& submitInfo);
	
	static void submitGraphicsQueue();

	static void getRenderingThreadID();

private:
	static std::queue<VkSubmitInfo> graphicsSubmitInfoQueue;
	static std::thread::id renderingThreadID;
	static std::condition_variable cv;
};
