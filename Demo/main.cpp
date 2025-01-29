#define GLFW_INCLUDE_VULKAN
#include <iostream>
#include <GLFW/glfw3.h>
#include <Smoothie.h>
#include <vector>


const std::vector<const char*> validationLayers =
{
"VK_LAYER_KHRONOS_validation"
};


const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 720;
GLFWwindow* window = nullptr;

int main()
{

	if (!glfwInit()) 
	{
		std::cout << "Can't initilize glfw!" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);

	//Vulkan extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	
	//vulkan instance
	auto instance = SmoothieCore::createVulkanInstance(extensions.data(), extensions.size(), true, validationLayers.data(), validationLayers.size());
	VkSurfaceKHR vksurface = nullptr;
	glfwCreateWindowSurface(instance, window, nullptr, &vksurface);

	auto gpuNames =SmoothieCore::getPhysicalDeviceNames(); 
	
	SmoothieMath::Vector3 cameraPosition = { -0.5, 3.0f, 18.0f };
	Smoothie::Camera camera = Smoothie::Camera(
		cameraPosition,
		{ 0.0f, 0.0f, -1.0f },
		{ 0.0f, 1.0f, 0.0f },
		45.0f, 1280/720, 0.1f, 100.0f);
	
	
	
	//Init engine
	SmoothieCore::initEngine(
		0, //Change index to the one you want from the gpuNames
		vksurface,
		WINDOW_WIDTH, 
		WINDOW_HEIGHT, 
		&camera);




	while (!glfwWindowShouldClose(window))
	{
		SmoothieCore::draw();
		glfwPollEvents();
	}

	SmoothieCore::finalize();
	glfwTerminate();
}