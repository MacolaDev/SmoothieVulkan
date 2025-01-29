#include "Renderer.h"
#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"
#include <iostream>
#include "DeferredPipeline.h"
#include "SwapChain.h"

VkCommandBuffer Renderer::renderCommandBuffer = nullptr;

VkSemaphore Renderer::imageAvailableSemaphore = nullptr;
VkSemaphore Renderer::renderFinishedSemaphore = nullptr;
VkFence Renderer::fence = nullptr;

void Renderer::createRenderer()
{

	//*********** Command buffer ******************//
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = SmoothieCore::getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(SmoothieCore::getDevice(), &allocInfo, &renderCommandBuffer);
	

	//********** Semaphores and fences ************//
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	vkCreateSemaphore(SmoothieCore::getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore);
	vkCreateSemaphore(SmoothieCore::getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore);
	
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	vkCreateFence(SmoothieCore::getDevice(), &fenceInfo, nullptr, &fence);

}

void Renderer::draw()
{
	vkWaitForFences(SmoothieCore::getDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
	vkResetFences(SmoothieCore::getDevice(), 1, &fence);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(SmoothieCore::getDevice(), SwapChain::getSwapChain(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	
	vkResetCommandBuffer(renderCommandBuffer, 0);

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(renderCommandBuffer, &commandBufferBeginInfo);
	DeferredPipeline::draw(renderCommandBuffer, imageIndex);
	vkEndCommandBuffer(renderCommandBuffer);
	
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &renderCommandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore;
	auto status = vkGetFenceStatus(SmoothieCore::getDevice(), fence);
	vkQueueSubmit(SmoothieCore::getGraphicsQueue(), 1, &submitInfo, fence);


	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
	
	auto swapchain = SwapChain::getSwapChain();
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(SmoothieCore::getPresentQueue(), &presentInfo);
}

void Renderer::destroyRenderer()
{
	vkQueueWaitIdle(SmoothieCore::getGraphicsQueue());
}