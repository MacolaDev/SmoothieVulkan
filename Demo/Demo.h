#pragma once
#include "Smoothie.h"
#include <GLFW/glfw3.h>
#include <optional>

struct FreeCamera 
{
	static Smoothie::Camera freeCamera;

	static void MouseCallback(GLFWwindow* window, double xposIn, double yposIn);
	static void KeyboardCallback(GLFWwindow* window);
	static void updateTime();
	static void resolutionUpdate(int width, int height);
};

class SmoothieEngineInitInfo : public Smoothie::SmoothieCore_Initialization
{
	VkDebugUtilsMessengerEXT debugMessanger = nullptr;

	std::optional<unsigned int> queueFamilyGraphicsIndex;
	std::optional<unsigned int> queueFamilyPresentIndex;

	unsigned int selected_device_index = 0;
	VkSurfaceFormatKHR surfaceFormat;

public:
	int create_instance(VkInstance& instance) override;
	int destroy_instance(VkInstance& instance) override;

	int create_surface(VkSurfaceKHR& surface) override;
	int destroy_surface(VkSurfaceKHR& surface) override;

	int select_physical_device(VkPhysicalDevice& physicalDevice) override;

	int create_device(VkDevice& device) override;
	int destroy_device(VkDevice& device) override;

	int getGraphicsQueue(VkQueue& queue) const override;
	int getGraphicsQueueFamilyIndex(unsigned int& queueFamilyIndex) const override;

	int getPresentQueue(VkQueue& queue) const override;
	int getPresentQueueFamilyIndex(unsigned int& queueFamilyIndex) const override;

	int getComputeQueue(VkQueue& queue) const override;
	int getComputeQueueFamilyIndex(unsigned int& queueFamilyIndex) const override;

	int create_swapchain(VkSwapchainKHR& swapchain) override;
	int destroy_swapchain(VkSwapchainKHR& swapchain) override;

	int get_swapchain_images(std::vector<VkImage>& images) override;
	int create_swapchain_image_views(std::vector<VkImageView>& imageViews) override;
	int destroy_swapchain_image_views(std::vector<VkImageView>& imageViews) override;

	int create_swapchain_framebuffers(std::vector<VkFramebuffer>& framebuffers) override;
	void destroy_swapchain_framebuffers(std::vector<VkFramebuffer>& framebuffers) override;

	int create_default_renderpass(VkRenderPass& renderPass) override;
	void destroy_default_renderpass(VkRenderPass& renderPass) override;

	GLFWwindow* window = nullptr;
};
