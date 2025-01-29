#pragma once
#include <vulkan/vulkan.h>

class Renderer
{
	static VkCommandBuffer renderCommandBuffer;

	static VkSemaphore imageAvailableSemaphore;
	static VkSemaphore renderFinishedSemaphore;
	static VkFence fence;
public:
	static void createRenderer();
	static void destroyRenderer();
	static void draw();

};

