#include "Renderer.h"
#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"
#include <iostream>
#include "DeferredPipeline.h"
#include "SwapChain.h"
#include "Core/Multithreading.h"

std::vector<VkCommandBuffer> Renderer::renderCommandBuffers;

std::vector<VkSemaphore> Renderer::imageAvailableSemaphores;
std::vector<VkSemaphore> Renderer::renderFinishedSemaphores;
std::vector<VkFence> Renderer::inFlightFences;

void Renderer::createRenderer()
{

	//*********** Command buffer ******************//
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = SmoothieCore::getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	renderCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkAllocateCommandBuffers(SmoothieCore::getDevice(), &allocInfo, &renderCommandBuffers[i]);
	}
	

	//********** Semaphores and fences ************//
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkCreateSemaphore(SmoothieCore::getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
		vkCreateSemaphore(SmoothieCore::getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
	}

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkCreateFence(SmoothieCore::getDevice(), &fenceInfo, nullptr, &inFlightFences[i]);
	}

}

static std::mutex renderMutex;

static unsigned int currentFrame = 0;
void Renderer::draw()
{
	vkWaitForFences(SmoothieCore::getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(SmoothieCore::getDevice(), 1, &inFlightFences[currentFrame]);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(SmoothieCore::getDevice(), SwapChain::getSwapChain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
	
	vkResetCommandBuffer(renderCommandBuffers[currentFrame], 0);

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(renderCommandBuffers[currentFrame], &commandBufferBeginInfo);
	DeferredPipeline::draw(renderCommandBuffers[currentFrame], imageIndex);
	vkEndCommandBuffer(renderCommandBuffers[currentFrame]);
	
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &renderCommandBuffers[currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

	vkQueueSubmit(SmoothieCore::getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]);

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
	
	auto swapchain = SwapChain::getSwapChain();
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;

	
	vkQueuePresentKHR(SmoothieCore::getPresentQueue(), &presentInfo);

	MultithreadSubmissions::submitGraphicsQueue();

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::destroyRenderer()
{
	vkQueueWaitIdle(SmoothieCore::getGraphicsQueue());
	
	for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyFence(SmoothieCore::getDevice(), inFlightFences[i], nullptr);
		inFlightFences[i] = nullptr;

		vkDestroySemaphore(SmoothieCore::getDevice(), renderFinishedSemaphores[i], nullptr);
		renderFinishedSemaphores[i] = nullptr;

		vkDestroySemaphore(SmoothieCore::getDevice(), imageAvailableSemaphores[i], nullptr);
		imageAvailableSemaphores[i] = nullptr;
	}

	
}