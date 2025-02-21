#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class Renderer
{
	static std::vector<VkCommandBuffer> renderCommandBuffers;

	static std::vector<VkSemaphore> imageAvailableSemaphores;
	static std::vector<VkSemaphore> renderFinishedSemaphores;
	static std::vector<VkFence> inFlightFences;

	const static unsigned int MAX_FRAMES_IN_FLIGHT = 2;

public:
	static void createRenderer();
	static void destroyRenderer();
	static void draw();

};

