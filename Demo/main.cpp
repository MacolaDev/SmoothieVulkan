#define GLFW_INCLUDE_VULKAN
#include <iostream>
#include <GLFW/glfw3.h>
#include <Smoothie.h>
#include <vector>
#include "Demo.h"

const std::vector<const char*> SMOOTHIE_VALIDATION_LAYERS =
{
	"VK_LAYER_KHRONOS_validation"
};

const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 720;

GLFWwindow* window = nullptr;

static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) 
{
	FreeCamera::MouseCallback(window, xposIn, yposIn);
}

static void resizeCallback(GLFWwindow* window, int width, int height) 
{
	FreeCamera::resolutionUpdate(width, height);
	SmoothieCore::updateRenderingResolution(width, height);
}

#ifdef _DEBUG
	constexpr bool useValidationlayers = true;
#else
	constexpr  bool useValidationlayers = false;
#endif

int main()
{

	if (!glfwInit()) 
	{
		std::cout << "Can't initilize glfw!" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "SmoothieVulkan", nullptr, nullptr);
	
	//Callback functions
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetFramebufferSizeCallback(window, resizeCallback);

	//Disable cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Vulkan extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	
	//vulkan instance
	auto instance = SmoothieCore::createVulkanInstance(extensions.data(), extensions.size(), useValidationlayers);
	VkSurfaceKHR vksurface = nullptr;
	glfwCreateWindowSurface(instance, window, nullptr, &vksurface);
	
	auto gpuNames = SmoothieCore::getPhysicalDeviceNames(); 
	
	//Init engine
	SmoothieCore::initEngine(
		1, //Change index to the one you want from the gpuNames
		vksurface,
		WINDOW_WIDTH, 
		WINDOW_HEIGHT);

	SmoothieCore::loadScene("resources/DemoScene/Demo.sscene");
	while (!glfwWindowShouldClose(window))
	{
		SmoothieCore::draw();
		FreeCamera::updateTime();
		FreeCamera::KeyboardCallback(window);
		glfwPollEvents();
	}

	SmoothieCore::removeScene();
	SmoothieCore::finalize();
	glfwTerminate();
}