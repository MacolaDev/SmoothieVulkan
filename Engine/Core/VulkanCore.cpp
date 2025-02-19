#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"
#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <Core/DeviceDependency.h>
#include "Smoothie.h"
VkInstance SmoothieCore::instance = nullptr;
VkPhysicalDevice SmoothieCore::physicalDevice = nullptr;
VkDevice SmoothieCore::device = nullptr;
VkDebugUtilsMessengerEXT SmoothieCore::debugMessenger = nullptr;
VkSurfaceKHR SmoothieCore::surface = nullptr;
VkCommandPool SmoothieCore::commandPool = nullptr;

std::optional<unsigned int> SmoothieCore::queueFamilyGraphicsIndex;
std::optional<unsigned int> SmoothieCore::queueFamilyPresentIndex;

VkQueue SmoothieCore::graphicsQueue = nullptr;
VkQueue SmoothieCore::presentQueue = nullptr;

const char* const* SmoothieCore::extensionNames = nullptr;
size_t SmoothieCore::extensionCount = 0;
bool SmoothieCore::useDebugging = false;



static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::string msg = pCallbackData->pMessage;
		std::cout << "\nSmoothie: Vulkan: \n" << msg << std::endl;
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
	}
	return VK_FALSE;
}


VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger
)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}


#include "Smoothie.h"
VkInstance SmoothieCore::createVulkanInstance(
	const char* const* extensionNames, 
	size_t extensionCount,
	bool useDebugging
)
{
	//Appending data
	SmoothieCore::extensionNames = extensionNames;
	SmoothieCore::extensionCount = extensionCount;
	SmoothieCore::useDebugging = useDebugging;
	
	//App creation
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Smoothie Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Smoothie";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	//Instance
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;
	if (useDebugging)
	{
		createInfo.enabledLayerCount = static_cast<unsigned int>(SMOOTHIE_VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = SMOOTHIE_VALIDATION_LAYERS.data();
	}
	
	createInfo.pNext = nullptr;
	createInfo.enabledExtensionCount = static_cast<unsigned int>(extensionCount);
	createInfo.ppEnabledExtensionNames = extensionNames;

	VkResult restult = vkCreateInstance(&createInfo, nullptr, &instance);
	if (restult != VK_SUCCESS)
	{
		return nullptr;
	}

	//Debugging 
	if (!useDebugging) return instance;
	
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	debugCreateInfo.sType =
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;


	debugCreateInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	debugCreateInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	debugCreateInfo.pfnUserCallback = debugCallback;

	
	if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		return nullptr;
	}

	return instance;
}

std::vector<std::string> SmoothieCore::getPhysicalDeviceNames()
{
	if (instance == nullptr)
	{
		std::cout << __FUNCTION__": Initilize Vulkan instance first!" << std::endl;
		return std::vector<std::string>();
	}

	//Physical devices
	unsigned int deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		std::cout << __FUNCTION__": No sutiable Vulkan device on this machine! Can't processed!" << std::endl;
		return std::vector<std::string>();
	}


	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	


	std::vector<std::string> deviceNames;
	for (int i = 0; i < devices.size(); i++)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(devices[i], &properties);
		deviceNames.push_back(properties.deviceName);
	}

	return deviceNames;
}

void SmoothieCore::initVulkan(unsigned int GPUIndex, VkSurfaceKHR surface)
{
	SmoothieCore::surface = surface;
	
	if (instance == nullptr) 
	{
		std::cout << __FUNCTION__": Initilize Vulkan instance first!" << std::endl;
		return;
	}

	//Physical device
	unsigned int deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		std::cout << __FUNCTION__": No sutiable Vulkan device on this machine!" << std::endl;
		return;
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	if (GPUIndex > (devices.size() - 1))
	{
		std::cout << __FUNCTION__": Invalid GPU Index! Can't find valid GPU!" << std::endl;
		return;
	}

	physicalDevice = devices[GPUIndex];
	
	//Getting dependencies
	DeviceDependencies::getDendepndenciesFromDevice(physicalDevice);


	//Queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueFamilyGraphicsIndex = i;
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
		if (presentSupport)
		{
			queueFamilyPresentIndex = i;
		}
		if (queueFamilyPresentIndex.has_value() && queueFamilyGraphicsIndex.has_value())
		{
			break;
		}
		i++;
	}


	//Logical device creation
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { queueFamilyGraphicsIndex.value(), queueFamilyPresentIndex.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	VkPhysicalDeviceFeatures deviceFeatures{};
	createInfo.pEnabledFeatures = &deviceFeatures;
	std::vector<const char*> deviceExtentions;
	deviceExtentions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	//deviceExtentions.push_back(VK_EXT_DEVICE_FAULT_EXTENSION_NAME);
	createInfo.enabledExtensionCount = static_cast<unsigned int>(deviceExtentions.size());
	createInfo.ppEnabledExtensionNames = deviceExtentions.data();

	VkPhysicalDeviceFaultFeaturesEXT faultFeatures{};
	faultFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT;
	faultFeatures.deviceFault = true;
	faultFeatures.deviceFaultVendorBinary = true;
	createInfo.pNext = nullptr;
	vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);

	//Set device graphic and present queue
	vkGetDeviceQueue(device, queueFamilyGraphicsIndex.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueFamilyPresentIndex.value(), 0, &presentQueue);
	
	//Command pool
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyGraphicsIndex.value();

	vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
}

VkInstance SmoothieCore::getInstance()
{
	return instance;
}

VkPhysicalDevice SmoothieCore::getPhysicalDevice()
{
	return physicalDevice;
}

VkDevice SmoothieCore::getDevice() 
{
	return device;
}

VkSurfaceKHR SmoothieCore::getSurface()
{
	return surface;
}

unsigned int SmoothieCore::getQueueFamilyGraphicsIndex()
{
	return queueFamilyGraphicsIndex.value();
}

unsigned int SmoothieCore::getQueueFamilyPresentIndex()
{
	return queueFamilyPresentIndex.value();
}

VkQueue SmoothieCore::getGraphicsQueue()
{
	return graphicsQueue;
}

VkQueue SmoothieCore::getPresentQueue()
{
	return presentQueue;
}

VkCommandPool SmoothieCore::getCommandPool()
{
	return commandPool;
}

static void destroyDebugMessanger(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator)
{	
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, messenger, pAllocator);
	}
	
}

void SmoothieCore::destroyEngine()
{
	vkDestroyCommandPool(device, commandPool, nullptr);
	commandPool = nullptr;

	vkDestroyDevice(device, nullptr);
	device = nullptr;

	destroyDebugMessanger(instance, debugMessenger, nullptr);
	debugMessenger = nullptr;

	vkDestroySurfaceKHR(instance, surface, nullptr);
	surface = nullptr;

	vkDestroyInstance(instance, nullptr);
	instance = nullptr;
}
