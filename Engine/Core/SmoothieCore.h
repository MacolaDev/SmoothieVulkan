#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <optional>
#include "UniformBuffers.h"
#include "Camera.h"
#include "Core/Scene.h"

class SmoothieCore
{
    friend class SmoothieVulkanCore;

    static bool isEngineReady;
    static Scene* scene;

public:
    static void initEngine(unsigned int GPUIndex, VkSurfaceKHR surface, unsigned int windowWidth, unsigned int windowHeight);
    static void finalize();
    static void draw();

    static VkInstance createVulkanInstance(const char* const* extensionNames, size_t extensionCount, bool useValidationLayers);

    static std::vector<std::string> getPhysicalDeviceNames();

    static void loadScene(const std::string& scene_file);
    static void removeScene();

    //Updates render engine with new camera data.
    static void updateCameraData(const Smoothie::Camera& camera);

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