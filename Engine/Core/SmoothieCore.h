#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <optional>
#include "UniformBuffers.h"
#include "Camera.h"

class SmoothieCore
{
    friend class SmoothieVulkanCore;

    static bool isEngineReady;
    static Smoothie::Camera* camera;

public:
    static void initEngine(unsigned int GPUIndex, VkSurfaceKHR surface, unsigned int windowWidth, unsigned int windowHeight, Smoothie::Camera* camera);
    static void finalize();
    static void draw();

    static VkInstance createVulkanInstance(const char* const* extensionNames, size_t extensionCount, bool useDebugging, const char* const* layerNames = nullptr, size_t layerCount = 0);

    static std::vector<std::string> getPhysicalDeviceNames();

//Conteains Vulkan-related stuff for engine
#ifdef _SMOOTHIE_ENGINE
private:
    static VkInstance instance;
    static VkPhysicalDevice physicalDevice;
    static VkDevice device;
    static VkDebugUtilsMessengerEXT debugMessenger;
    static VkSurfaceKHR surface;
    static VkCommandPool commandPool;

    static std::optional<unsigned int> queueFamilyGraphicsIndex;
    static std::optional<unsigned int> queueFamilyPresentIndex;

    static VkQueue graphicsQueue;
    static VkQueue presentQueue;

    static const char* const* extensionNames;
    static size_t extensionCount;
    static bool useDebugging;
    static const char* const* layerNames;
    static size_t layerCount;


    //Creates physical device and logical device.
    static void initVulkan(unsigned int GPUIndex, VkSurfaceKHR surface);

public:

    static VkInstance getInstance();
    static VkPhysicalDevice getPhysicalDevice();
    static VkDevice getDevice();
    
    static VkSurfaceKHR getSurface();

    static unsigned int getQueueFamilyGraphicsIndex();
    static unsigned int getQueueFamilyPresentIndex();

    static VkQueue getGraphicsQueue();
    static VkQueue getPresentQueue();
    static VkCommandPool getCommandPool();
    
    static int SCR_WIDTH, SCR_HEIGHT;
#endif
};