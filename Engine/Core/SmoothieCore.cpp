#include "SmoothieCore.h"
#include <vulkan/vulkan.h>

#include <iostream>
#include <random>

#include "Core/Shader.h"
#include "Core/RenderPass.h"
#include "Core/CameraDescriptor.h"
#include "Core/Multithreading.h"

#include "Effects/DeferredPasses.h"
#include "Effects/DeferredPipeline.h"
using namespace Smoothie;

bool SmoothieCore::isEngineReady = false;

unsigned int SmoothieCore::SCR_WIDTH = 1280;
unsigned int SmoothieCore::SCR_HEIGHT = 720;

constexpr float apsect_ratio = static_cast<float>(1280.0f / 720.0f);
const Smoothie::Camera defaultCamera = Smoothie::Camera(
	{ -0.5, 3.0f, 18.0f }, 
	{ 0.0f, 0.0f, -1.0f },
	{ 0.0f, 1.0f, 0.0f },
	45.0f, apsect_ratio, 0.1f, 100.0f);

VkSwapchainKHR SmoothieCore::swapchain = nullptr;
std::vector<VkImage> SmoothieCore::swapchainImages;
std::vector<VkImageView> SmoothieCore::swapchainImageViews;

std::shared_ptr<Smoothie::Scene_Default> SmoothieCore::scene = nullptr;
std::shared_ptr<Smoothie::SmoothieCore_Initialization> SmoothieCore::init_info = nullptr;

VkInstance SmoothieCore::instance = nullptr;
VkPhysicalDevice SmoothieCore::physicalDevice = nullptr;
VkDevice SmoothieCore::device = nullptr;
VkSurfaceKHR SmoothieCore::surface = nullptr;
VkCommandPool SmoothieCore::commandPool = nullptr;

unsigned int SmoothieCore::queueFamilyGraphicsIndex = -1;
unsigned int SmoothieCore::queueFamilyPresentIndex = -1;

VkQueue SmoothieCore::graphicsQueue = nullptr;
VkQueue SmoothieCore::presentQueue = nullptr;

VmaAllocator SmoothieCore::vmaAllocator = nullptr;
static int create_vulkan_allocator(VmaAllocator& allocator)
{
	VmaVulkanFunctions vulkanFunctions = {};
	vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
	allocatorCreateInfo.physicalDevice = SmoothieCore::getPhysicalDevice();
	allocatorCreateInfo.device = SmoothieCore::getDevice();
	allocatorCreateInfo.instance = SmoothieCore::getInstance();
	allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
	
	if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS)
	{
		return 1;
	}
	return 0;
}

std::vector<CameraDescriptorSet> SmoothieCore::cameraDescriptorSets;

Smoothie::DefaultTexture2D SmoothieCore::default2dTexture;
Smoothie::DefaultBuffer SmoothieCore::defaultBuffer;
std::shared_ptr<Smoothie::Drawing_Base> SmoothieCore::drawerClass;

int SmoothieCore::initEngine
(
	std::shared_ptr<Smoothie::SmoothieCore_Initialization>& initInfo, 
	unsigned int windowWidth, unsigned windowHeight,
	std::shared_ptr<Smoothie::Scene_Default>& scene_loader,
	std::shared_ptr<Smoothie::Drawing_Base> drawingClass
)
{
	scene = scene_loader;
	

	init_info = initInfo;
	SCR_WIDTH = windowWidth, SCR_HEIGHT = windowHeight;
	MultithreadSubmissions::getRenderingThreadID();

	if (initInfo.get() == nullptr)
	{
		std::cout << "Initialization info must be a valid pointer to a SmoothieCore_Initialization child class!" << std::endl;
		return 1;
	}

	if (initInfo->create_instance(instance) != 0)
	{
		std::cout << "Failed to create Vulkan instance!" << std::endl;
		return 1;
	}

	if (initInfo->select_physical_device(physicalDevice))
	{
		std::cout << "Failed to select physical device!" << std::endl;
		return 1;
	}

	if (initInfo->create_surface(surface) != 0)
	{
		std::cout << "Failed to create Vulkan surface!" << std::endl;
		return 1;
	}

	if (initInfo->create_device(device) != 0)
	{
		std::cout << "Failed to create Vulkan device!" << std::endl;
		return 1;
	}

	if ((initInfo->getGraphicsQueue(graphicsQueue) != 0) || (initInfo->getGraphicsQueueFamilyIndex(queueFamilyGraphicsIndex) != 0))
	{
		std::cout << "Could not get the graphics queue or graphics queue family index!" << std::endl;
		return 1;
	}

	if ((initInfo->getPresentQueue(presentQueue) != 0) || (initInfo->getPresentQueueFamilyIndex(queueFamilyPresentIndex) != 0))
	{
		std::cout << "Could not get the present queue or present queue family index!" << std::endl;
		return 1;
	}

	/*if ((initInfo.getComputeQueue(graphicsQueue) != 0) || (initInfo.getComputeQueueFamilyIndex(queueFamilyGraphicsIndex) != 0))
	{
		std::cout << "Could not get the compute queue or compute queue family index!" << std::endl;
		return 1;
	}*/

	if (initInfo->create_swapchain(swapchain) != 0)
	{
		std::cout << "Failed to create Swapchain!" << std::endl;
		return 1;
	}

	if (initInfo->create_default_renderpass(defaultRenderPass) != 0)
	{
		std::cout << "Failed to create default render pass!" << std::endl;
		return 1;
	}

	if (initInfo->get_swapchain_images(swapchainImages))
	{
		std::cout << "Failed to get swapchain images!" << std::endl;
		return 1;
	}

	if (initInfo->create_swapchain_image_views(swapchainImageViews))
	{
		std::cout << "Failed to create swapchain image views!" << std::endl;
		return 1;
	}

	if (initInfo->create_swapchain_framebuffers(swapchainFramebuffers) != 0)
	{
		std::cout << "Failed to create swapchain framebuffers" << std::endl;
		return 1;
	}


	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyGraphicsIndex;
	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		std::cout << "Failed to create main command pool!" << std::endl;
		return 1;
	}

	if (create_vulkan_allocator(vmaAllocator) != 0)
	{
		std::cout << "Failed to create vulkan allocator!" << std::endl;
		return 1;
	}

	if (default2dTexture.create() != 0)
	{
		std::cout << "Failed to create default 2D texture image or image view!" << std::endl;
		return 1;
	}

	if (defaultBuffer.create() != 0)
	{
		std::cout << "Failed to create default buffer!" << std::endl;
		return 1;
	}


	cameraDescriptorSets.resize(SMOOTHIE_MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < SMOOTHIE_MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (cameraDescriptorSets[i].create() != 0)
		{
			std::cout << "Failed to create Camera descriptor sets!" << std::endl;
			return 1;
		}
		cameraDescriptorSets[i].update_camera_data(defaultCamera.getCameraBufferData());
	}

	if (create_renderer() != 0)
	{
		std::cout << "Failed to create renderer!" << std::endl;
	}

	drawerClass = drawingClass;
	if (drawerClass->create() != 0)
	{
		std::cout << "Failed to create drawing data!" << std::endl;
		return 1;
	}


	isEngineReady = true;
	return 0;
}

static void destroy_system_shaders(std::unordered_map<std::string, VkShaderModule>& shaders)
{
	for (auto& [key, value] : shaders) 
	{
		vkDestroyShaderModule(SmoothieCore::getDevice(), value, nullptr);
		value = nullptr;
	}
}

int SmoothieCore::finitEngine()
{
	removeScene();
	
	if (drawerClass != nullptr) drawerClass->destroy();

	destroy_renderer();

	for (size_t i = 0; i < cameraDescriptorSets.size(); i++)
	{
		cameraDescriptorSets[i].destroy();
	}
	
	defaultBuffer.destroy();
	default2dTexture.destroy();

	/*char* statsStr = nullptr;
	vmaBuildStatsString(SmoothieCore::getVulkanMemoryAllocator(), &statsStr, VK_TRUE);
	std::cout << statsStr << std::endl;
	vmaFreeStatsString(SmoothieCore::getVulkanMemoryAllocator(), statsStr);*/

	vmaDestroyAllocator(vmaAllocator);
	vmaAllocator = nullptr;
	
	vkDestroyCommandPool(SmoothieCore::getDevice(), commandPool, nullptr);
	commandPool = nullptr;

	init_info->destroy_swapchain_framebuffers(swapchainFramebuffers);
	init_info->destroy_swapchain_image_views(swapchainImageViews);
	init_info->destroy_default_renderpass(defaultRenderPass);
	init_info->destroy_swapchain(swapchain);
	init_info->destroy_device(device);
	init_info->destroy_surface(surface);
	init_info->destroy_instance(instance);

	return 0;
}

static bool isSceneLoaded = false;
int SmoothieCore::loadScene(const std::string& scene_file)
{
	auto _scene = scene.get();
	if ((_scene == nullptr) || (isEngineReady == false))
	{
		std::cout << "Can't load scene because engine is not initilized!" << std::endl;
		return 1;
	}

	if (isSceneLoaded == true)
	{
		std::cout << "Scene is already loaded!" << std::endl;
		return 1;
	}

	return _scene->load_scene(scene_file);
}

void SmoothieCore::removeScene()
{
	auto _scene = scene.get();
	if ((_scene == nullptr) || (isEngineReady == false)) return;
	_scene->clear_scene();
}

void SmoothieCore::updateCameraData(const Smoothie::Camera& camera)
{
	auto uniformBufferData = camera.getCameraBufferData();
	uniformBufferData.SCR_WIDTH = SCR_WIDTH;
	uniformBufferData.SCR_HEIGHT = SCR_HEIGHT;
	cameraDescriptorSets[currentFrame].update_camera_data(uniformBufferData);
}

void SmoothieCore::updateRenderingResolution(unsigned int windowWidth, unsigned int windowHeight)
{
	vkDeviceWaitIdle(device);
	if (windowWidth < 2 || windowHeight < 2) 
	{ 
		isEngineReady = false; 
		return; 
	}
	else
	{
		SCR_WIDTH = windowWidth, SCR_HEIGHT = windowHeight;
		isEngineReady = true;

		init_info->destroy_swapchain_framebuffers(swapchainFramebuffers);
		init_info->destroy_swapchain_image_views(swapchainImageViews);
		init_info->destroy_default_renderpass(defaultRenderPass);
		init_info->destroy_swapchain(swapchain);

		if (init_info->create_swapchain(swapchain) != 0)
		{
			std::cout << "Failed to create Swapchain!" << std::endl;
		}

		if (init_info->create_default_renderpass(defaultRenderPass) != 0)
		{
			std::cout << "Failed to create default render pass!" << std::endl;
		}

		if (init_info->get_swapchain_images(swapchainImages))
		{
			std::cout << "Failed to get swapchain images!" << std::endl;
		}

		if (init_info->create_swapchain_image_views(swapchainImageViews))
		{
			std::cout << "Failed to create swapchain image views!" << std::endl;
		}

		if (init_info->create_swapchain_framebuffers(swapchainFramebuffers) != 0)
		{
			std::cout << "Failed to create swapchain framebuffers" << std::endl;
		}

		if (drawerClass->resize_callback() != 0)
		{
			isEngineReady = false;
		}
	}
}

unsigned int SmoothieCore::generate_random_key()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<unsigned int> dist(1, std::numeric_limits<unsigned int>::max());
	return dist(gen);
}

std::array<VkCommandBuffer, SMOOTHIE_MAX_FRAMES_IN_FLIGHT> SmoothieCore::renderCommandBuffers;
std::array<VkSemaphore, SMOOTHIE_MAX_FRAMES_IN_FLIGHT> SmoothieCore::imageAvailableSemaphores;
std::array<VkSemaphore, SMOOTHIE_MAX_FRAMES_IN_FLIGHT> SmoothieCore::renderFinishedSemaphores;
std::array<VkFence, SMOOTHIE_MAX_FRAMES_IN_FLIGHT> SmoothieCore::inFlightFences;

std::vector<VkFramebuffer> SmoothieCore::swapchainFramebuffers;
VkRenderPass SmoothieCore::defaultRenderPass = nullptr;

unsigned int SmoothieCore::currentFrame = 0;


int SmoothieCore::create_renderer()
{
	//*********** Command buffer ******************//
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = SmoothieCore::getMainCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	for (unsigned int i = 0; i < SMOOTHIE_MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkAllocateCommandBuffers(SmoothieCore::getDevice(), &allocInfo, &renderCommandBuffers[i]) != VK_SUCCESS) 
		{ 
			return 1; 
		}
	}


	//********** Semaphores and fences ************//
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	for (unsigned int i = 0; i < SMOOTHIE_MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(SmoothieCore::getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS)
		{
			return 1;
		}
		if (vkCreateSemaphore(SmoothieCore::getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
		{
			return 1;
		}
	}

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (unsigned int i = 0; i < SMOOTHIE_MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateFence(SmoothieCore::getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			return 1;
		}
	}

	return 0;
}

int SmoothieCore::destroy_renderer()
{
	vkQueueWaitIdle(SmoothieCore::getGraphicsQueue());

	for (unsigned int i = 0; i < SMOOTHIE_MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyFence(SmoothieCore::getDevice(), inFlightFences[i], nullptr);
		inFlightFences[i] = nullptr;

		vkDestroySemaphore(SmoothieCore::getDevice(), renderFinishedSemaphores[i], nullptr);
		renderFinishedSemaphores[i] = nullptr;

		vkDestroySemaphore(SmoothieCore::getDevice(), imageAvailableSemaphores[i], nullptr);
		imageAvailableSemaphores[i] = nullptr;
	}
	return 0;
}

void SmoothieCore::draw()
{
	if ((scene.get() == nullptr) || (isEngineReady == false)) return;
	

	vkWaitForFences(SmoothieCore::getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(SmoothieCore::getDevice(), 1, &inFlightFences[currentFrame]);

	uint32_t imageIndex = 0;
	vkAcquireNextImageKHR(SmoothieCore::getDevice(), getSwapchain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(renderCommandBuffers[currentFrame], 0);

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(renderCommandBuffers[currentFrame], &commandBufferBeginInfo);
	if(drawerClass != nullptr) drawerClass->draw(renderCommandBuffers[currentFrame], imageIndex);
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

	auto swapchain = getSwapchain();
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;


	vkQueuePresentKHR(SmoothieCore::getPresentQueue(), &presentInfo);
	MultithreadSubmissions::submitGraphicsQueue();
	currentFrame = (currentFrame + 1) % SMOOTHIE_MAX_FRAMES_IN_FLIGHT;
}

