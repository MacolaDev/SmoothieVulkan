//
// Created by macola on 7/15/26.
//
#include "init_vulkan.h"
#include <set>

using namespace Smoothie::App_Utilities;


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::string_view _type;
    switch (messageType)
    {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: _type = "General"; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: _type = "Validation"; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: _type = "Performance"; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT: _type = "AddressBindingBit"; break;
        default: _type = "Unknown"; break;
    }

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        SmoothieCore::logError(fmt::format("[VULKAN] Type: {}; Message: {}", _type, pCallbackData->pMessage));
        return VK_FALSE;
    }

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        SmoothieCore::logWarning(fmt::format("[VULKAN] Type: {}; Message: {}", _type, pCallbackData->pMessage));
        return VK_FALSE;
    }

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        SmoothieCore::logInfo(fmt::format("[VULKAN] Type: {}; Message: {}", _type, pCallbackData->pMessage));
        return VK_FALSE;
    }

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        SmoothieCore::logVerbose(fmt::format("[VULKAN]: Type: {}; Message: {}", _type, pCallbackData->pMessage));
        return VK_FALSE;
    }

    return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, nullptr, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static VkDebugUtilsMessengerEXT g_DebugUtilsMessenger = nullptr;

int Smoothie::App_Utilities::create_instance(Smoothie::EngineInitInfo& init_info, bool useValidationLayers)
{
    assert(init_info.instance == nullptr);
    if (init_info.instance != nullptr) return 0;

    VkApplicationInfo _application_info{};
    _application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    _application_info.pApplicationName = "Smoothie Engine";
    _application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    _application_info.pEngineName = "Smoothie";
    _application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    _application_info.apiVersion = SmoothieCore::getRequiredVulkanAPIVersion();

    //Instance
    VkInstanceCreateInfo _instance_create_info{};
    _instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    _instance_create_info.pNext = nullptr;
    _instance_create_info.pApplicationInfo = &_application_info;


    std::vector<const char*> _validation_layers;
    if (useValidationLayers)
    {
        _validation_layers.push_back("VK_LAYER_KHRONOS_validation");
    }
    _instance_create_info.enabledLayerCount = static_cast<unsigned int>(_validation_layers.size());
    _instance_create_info.ppEnabledLayerNames = _validation_layers.data();
    SmoothieCore::logInfo(fmt::format("Enabled layers: {}; {}", _validation_layers.size(), fmt::join(_validation_layers, ", ")));

    //Vulkan extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector _extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (useValidationLayers)
    {
        _extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    _instance_create_info.enabledExtensionCount = static_cast<unsigned int>(_extensions.size());
    _instance_create_info.ppEnabledExtensionNames = _extensions.data();

    SmoothieCore::logInfo(fmt::format("Enabled extension: {}; {}", _extensions.size(), fmt::join(_extensions, ", ")));

    if (VkResult result = vkCreateInstance(&_instance_create_info, nullptr, &init_info.instance); result != VK_SUCCESS)
    {
        SmoothieCore::logError(fmt::format("Failed to create vulkan instance!, return code: {}", static_cast<unsigned int>(result)));
        return 1;
    }

    if (useValidationLayers)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType =
            //VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;

        if (CreateDebugUtilsMessengerEXT(init_info.instance, &debugCreateInfo, &g_DebugUtilsMessenger) != VK_SUCCESS)
        {
            SmoothieCore::logError("Failed to create debug manager!");
            return 1;
        }
    }

    return 0;
}

void Smoothie::App_Utilities::destroy_instance(Smoothie::EngineInitInfo& init_info)
{
    if (init_info.instance == nullptr) return;

    if (g_DebugUtilsMessenger != nullptr)
    {
        auto _destroy_func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(init_info.instance, "vkDestroyDebugUtilsMessengerEXT");
        if (_destroy_func == nullptr) return;
        _destroy_func(init_info.instance, g_DebugUtilsMessenger, nullptr);
    }

    vkDestroyInstance(init_info.instance, nullptr);
    init_info.instance = nullptr;
}

int Smoothie::App_Utilities::create_surface(EngineInitInfo& init_info, GLFWwindow* window)
{
    assert(init_info.instance != nullptr);
    assert(window != nullptr);

    if (glfwCreateWindowSurface(init_info.instance, window, nullptr, &init_info.surface) != VK_SUCCESS)
    {
        SmoothieCore::logError("Failed to create GLFW surface!");
        return 1;
    }

    return 0;
}

void Smoothie::App_Utilities::destroy_surface(EngineInitInfo& init_info)
{
    assert(init_info.instance != nullptr);
    if (init_info.surface != nullptr)
    {
        vkDestroySurfaceKHR(init_info.instance, init_info.surface, nullptr);
        init_info.surface = nullptr;
    }
}

int Smoothie::App_Utilities::select_physical_device(Smoothie::EngineInitInfo &init_info, unsigned int device_index)
{
    assert(init_info.instance != nullptr);

    unsigned int _count = 0;
    if (vkEnumeratePhysicalDevices(init_info.instance, &_count, nullptr) != VK_SUCCESS)
    {
        SmoothieCore::logError("No suitable Vulkan devices on this machine!");
        return 1;
    }

    std::vector<VkPhysicalDevice> _devices(_count);
    if (vkEnumeratePhysicalDevices(init_info.instance, &_count, _devices.data()) != VK_SUCCESS)
    {
        SmoothieCore::logError("No suitable Vulkan devices on this machine!");
        return 1;
    }

    assert(device_index < _count);
    if (device_index > _count) return 1;

    init_info.physical_device = _devices[device_index];

    std::vector<std::string> _device_names(_count);
    for (unsigned int i = 0; i < _count; i++)
    {
        VkPhysicalDeviceProperties _device_property{};
        vkGetPhysicalDeviceProperties(_devices[i], &_device_property);
        _device_names[i] = _device_property.deviceName;
    }

    SmoothieCore::logVerbose(fmt::format("Physical devices: {}; {}", _count, fmt::join(_device_names, ", ")));

    VkPhysicalDeviceProperties _selected_device_properties{};
    vkGetPhysicalDeviceProperties(init_info.physical_device, &_selected_device_properties);
    SmoothieCore::logInfo(fmt::format("Selected physical device: {}", _selected_device_properties.deviceName));
    SmoothieCore::logVerbose(fmt::format("Physical device info: Driver version: {}, VendorID: {:#x}", _selected_device_properties.driverVersion, _selected_device_properties.vendorID));

    //TODO: Check additional support for a specific device (like Vulkan version)

    return 0;
}

int Smoothie::App_Utilities::create_device_and_queues(Smoothie::EngineInitInfo &init_info)
{
    assert(init_info.physical_device != nullptr);
    assert(init_info.surface != nullptr);
    assert(init_info.device == nullptr);

    //Queue families
    uint32_t _queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(init_info.physical_device, &_queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> _queue_families(_queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(init_info.physical_device, &_queue_family_count, _queue_families.data());

    std::optional<unsigned int> _queue_family_index_graphics;
    std::optional<unsigned int> _queue_family_index_present;

    int i = 0;
    for (const auto& _family : _queue_families)
    {
        if (_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            _queue_family_index_graphics = i;
        }
        VkBool32 _presentation_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(init_info.physical_device, i, init_info.surface, &_presentation_support);
        if (_presentation_support)
        {
            _queue_family_index_present = i;
        }
        if (_queue_family_index_present.has_value() && _queue_family_index_graphics.has_value())
        {
            break;
        }
        i++;
    }


    //Logical device creation
    std::vector<VkDeviceQueueCreateInfo> _queue_create_infos;
    std::set _unique_queue_families = { _queue_family_index_graphics.value(), _queue_family_index_present.value() };

    float _priority = 1.0f;
    for (uint32_t _family : _unique_queue_families)
    {
        VkDeviceQueueCreateInfo _queue_create_info{};
        _queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        _queue_create_info.queueFamilyIndex = _family;
        _queue_create_info.queueCount = 1;
        _queue_create_info.pQueuePriorities = &_priority;
        _queue_create_infos.push_back(_queue_create_info);
    }

    VkDeviceCreateInfo _device_create_info{};
    _device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    _device_create_info.queueCreateInfoCount = static_cast<uint32_t>(_queue_create_infos.size());
    _device_create_info.pQueueCreateInfos = _queue_create_infos.data();

    VkPhysicalDeviceFeatures _device_features{};
    SmoothieCore::getRequiredPhysicalDeviceFeatures(_device_features);
    _device_create_info.pEnabledFeatures = &_device_features;

    std::vector<const char*> _device_extensions;
    SmoothieCore::getRequiredDeviceExtensions(_device_extensions);

    _device_create_info.enabledExtensionCount = static_cast<unsigned int>(_device_extensions.size());
    _device_create_info.ppEnabledExtensionNames = _device_extensions.data();

    VkPhysicalDeviceVulkan11Features _features11{};
    SmoothieCore::getRequiredPhysicalDeviceFeatures_Vk11(_features11);
    _features11.pNext = nullptr;

    VkPhysicalDeviceVulkan12Features _features12{};
    SmoothieCore::getRequiredPhysicalDeviceFeatures_Vk12(_features12);
    _features12.pNext = &_features11;

    VkPhysicalDeviceVulkan13Features _features13{};
    SmoothieCore::getRequiredPhysicalDeviceFeatures_Vk13(_features13);
    _features13.pNext = &_features12;

    VkPhysicalDeviceVulkan14Features _features14{};
    SmoothieCore::getRequiredPhysicalDeviceFeatures_Vk14(_features14);
    _features14.pNext = &_features13;

    _device_create_info.pNext = &_features14;
    if (vkCreateDevice(init_info.physical_device, &_device_create_info, nullptr, &init_info.device) != VK_SUCCESS)
    {
        SmoothieCore::logError("Failed to create VkDevice!");
        return 1;
    }


    assert(init_info.queue_family_index_graphics == -1);
    assert(init_info.queue_graphics == nullptr);

    init_info.queue_family_index_graphics = _queue_family_index_graphics.value();
    vkGetDeviceQueue(init_info.device, _queue_family_index_graphics.value(), 0, &init_info.queue_graphics);

    assert(init_info.queue_family_index_present == -1);
    assert(init_info.queue_present == nullptr);

    init_info.queue_family_index_present = _queue_family_index_present.value();
    vkGetDeviceQueue(init_info.device, _queue_family_index_present.value(), 0, &init_info.queue_present);

    init_info.queue_family_index_worker = init_info.queue_family_index_graphics;
    init_info.queue_worker = init_info.queue_graphics;

    //vkGetDeviceQueue(SmoothieCore::getDevice(), queueFamilyPresentIndex.value(), 0, &queue); #Worked queue

    return 0;
}

void Smoothie::App_Utilities::destroy_device_and_queues(Smoothie::EngineInitInfo &init_info)
{
    if (init_info.device != nullptr)
    {
        vkDestroyDevice(init_info.device, nullptr);
        init_info.device = nullptr;
    }
}

static VkSurfaceFormatKHR select_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

static VkPresentModeKHR select_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

int Smoothie::App_Utilities::create_swapchain(EngineInitInfo &init_info, unsigned int width, unsigned int height)
{
    assert(init_info.physical_device != nullptr);
    assert(init_info.surface != nullptr);
    assert(width > 0 && height > 0);
    assert(init_info.device != nullptr);

    VkSurfaceCapabilitiesKHR _capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(init_info.physical_device, init_info.surface, &_capabilities);

    uint32_t _format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(init_info.physical_device, init_info.surface, &_format_count, nullptr);

    std::vector<VkSurfaceFormatKHR> _formats;
    std::vector<VkPresentModeKHR> _present_modes;
    if (_format_count != 0)
    {
        _formats.resize(_format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(init_info.physical_device, init_info.surface, &_format_count, _formats.data());
    }

    uint32_t _present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(init_info.physical_device, init_info.surface, &_present_mode_count, nullptr);
    if (_present_mode_count != 0)
    {
        _present_modes.resize(_present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(init_info.physical_device, init_info.surface, &_present_mode_count, _present_modes.data());
    }

    auto _surface_format = select_surface_format(_formats);
    auto _present_mode = select_present_mode(_present_modes);

    VkExtent2D _extent{};
    _extent.width = width;
    _extent.height = height;

    uint32_t _image_count = _capabilities.minImageCount + 2;
    if (_capabilities.maxImageCount > 0 && _image_count > _capabilities.maxImageCount)
    {
        _image_count = _capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR _swapchain_create_info{};
    _swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    _swapchain_create_info.surface = init_info.surface;
    _swapchain_create_info.minImageCount = _image_count;
    _swapchain_create_info.imageFormat = _surface_format.format;
    _swapchain_create_info.imageColorSpace = _surface_format.colorSpace;
    _swapchain_create_info.imageExtent = _extent;
    _swapchain_create_info.imageArrayLayers = 1;
    _swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    _swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    const uint32_t queueFamilyIndices[] = { init_info.queue_family_index_graphics, init_info.queue_family_index_present };
    if (init_info.queue_family_index_graphics != init_info.queue_family_index_present)
    {
        _swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        _swapchain_create_info.queueFamilyIndexCount = 2;
        _swapchain_create_info.pQueueFamilyIndices = queueFamilyIndices;
    }

    _swapchain_create_info.preTransform = _capabilities.currentTransform;
    _swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    _swapchain_create_info.presentMode = _present_mode;
    _swapchain_create_info.clipped = VK_TRUE;
    _swapchain_create_info.oldSwapchain = init_info.swapchain;
    if (vkCreateSwapchainKHR(init_info.device, &_swapchain_create_info, nullptr, &init_info.swapchain) != VK_SUCCESS)
    {
        SmoothieCore::logError("Failed to create swapchain.");
        return 1;
    }


    init_info.swapchain_image_count = _image_count;
    init_info.swapchain_image_format = _surface_format.format;

    return 0;
}

void Smoothie::App_Utilities::destroy_swapchain(EngineInitInfo &init_info)
{
    if (init_info.swapchain != nullptr)
    {
        vkDestroySwapchainKHR(init_info.device, init_info.swapchain, nullptr);
        init_info.swapchain = nullptr;
    }
}
