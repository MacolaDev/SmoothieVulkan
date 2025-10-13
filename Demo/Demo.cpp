#include "Demo.h"
#include <set>

using namespace SmoothieMath;
constexpr float apsect_ratio = static_cast<float>(1280.0f / 720.0f);
Smoothie::Camera FreeCamera::freeCamera = Smoothie::Camera(
    { -0.0, 3.0f, 18.0f },
    { 0.0f, 0.0f, -1.0f },
    { 0.0f, 1.0f, 0.0f },
    45.0f, apsect_ratio, 0.1f, 100.0f);

float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;

float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;
float fov = 45.0f;
bool firstMouse = true;

void FreeCamera::MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    SmoothieMath::Vector3 front;
    front.x = cos(toRadians(yaw)) * cos(toRadians(pitch));
    front.y = sin(toRadians(pitch));
    front.z = sin(toRadians(yaw)) * cos(toRadians(pitch));
    front.normalizeVector();
    freeCamera.setCameraFront(front);
    freeCamera.updateCameraMatrices();
    SmoothieCore::updateCameraData(freeCamera);
}

static float cameraSpeed = 3;
static float deltaTime = 0.0f;
float currentFrame = 0.0f;
float lastFrame = 0.0f;

void FreeCamera::KeyboardCallback(GLFWwindow* window)
{
    //Press ESC to close Demo
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    SmoothieMath::Vector3 cameraPosition = freeCamera.getCameraPosition();

    float speed = cameraSpeed * deltaTime; // adjust accordingly

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPosition += freeCamera.getCameraFront() * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {

        cameraPosition -= freeCamera.getCameraFront() * speed;
    }


    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        auto d = cross(freeCamera.getCameraFront(), freeCamera.getCameraUp());
        cameraPosition = cameraPosition - (normalize(d) * speed);
    }


    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        auto b = cross(freeCamera.getCameraFront(), freeCamera.getCameraUp());
        cameraPosition = cameraPosition + (normalize(b) * speed);
    }

    freeCamera.setCameraPosition(cameraPosition);
    freeCamera.updateCameraMatrices();
    SmoothieCore::updateCameraData(freeCamera);
}

void FreeCamera::updateTime()
{
    currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void FreeCamera::resolutionUpdate(int width, int height)
{
    const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    freeCamera.setAspectRatio(aspectRatio);
    SmoothieCore::updateCameraData(freeCamera);
}


static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
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

int SmoothieEngineInitInfo::create_instance(VkInstance& instance)
{

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

#ifdef _DEBUG
    const std::vector<const char*> SMOOTHIE_VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
    createInfo.enabledLayerCount = static_cast<unsigned int>(SMOOTHIE_VALIDATION_LAYERS.size());
    createInfo.ppEnabledLayerNames = SMOOTHIE_VALIDATION_LAYERS.data();
#else
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
#endif // DEBUG


    //Vulkan extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    createInfo.pNext = nullptr;
    createInfo.enabledExtensionCount = static_cast<unsigned int>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {

        return 1;
    }


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
    
    if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessanger) != VK_SUCCESS)
    {
        return 1;
    }

    return 0;
}

static void destroyDebugMessanger(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, messenger, pAllocator);
    }

}

int SmoothieEngineInitInfo::destroy_instance(VkInstance& instance)
{

    destroyDebugMessanger(instance, debugMessanger, nullptr);
    debugMessanger = nullptr;

    vkDestroyInstance(instance, nullptr);
    instance = nullptr;
    return 0;
}

int SmoothieEngineInitInfo::create_surface(VkSurfaceKHR& surface)
{
    if (glfwCreateWindowSurface(SmoothieCore::getInstance(), window, nullptr, &surface) != VK_SUCCESS)
    {
        std::cout << "Failed to create GLFW surface!" << std::endl;
        return 1;
    }
    return 0;
}

int SmoothieEngineInitInfo::destroy_surface(VkSurfaceKHR& surface)
{
    vkDestroySurfaceKHR(SmoothieCore::getInstance(), surface, nullptr);
    return 0;
}

int SmoothieEngineInitInfo::select_physical_device(VkPhysicalDevice& physicalDevice)
{
    unsigned int deviceCount = 0;
    if (vkEnumeratePhysicalDevices(SmoothieCore::getInstance(), &deviceCount, nullptr) != VK_SUCCESS)
    {
        std::cout << "No sutiable Vulkan device on this machine!" << std::endl;
        return 1;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    if (vkEnumeratePhysicalDevices(SmoothieCore::getInstance(), &deviceCount, devices.data()) != VK_SUCCESS)
    {
        std::cout << "No sutiable Vulkan device on this machine!" << std::endl;
        return 1;
    }

    physicalDevice = devices[selected_device_index];
    return 0;
}

int SmoothieEngineInitInfo::create_device(VkDevice& device)
{
    //Queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(SmoothieCore::getPhysicalDevice(), &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(SmoothieCore::getPhysicalDevice(), &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueFamilyGraphicsIndex = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(SmoothieCore::getPhysicalDevice(), i, SmoothieCore::getSurface(), &presentSupport);
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
    for (uint32_t queueFamily : uniqueQueueFamilies) 
    {
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

    createInfo.enabledExtensionCount = static_cast<unsigned int>(deviceExtentions.size());
    createInfo.ppEnabledExtensionNames = deviceExtentions.data();
    
    if (vkCreateDevice(SmoothieCore::getPhysicalDevice(), &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        std::cout << "Could not create a device!" << std::endl;
        return 1;
    }
    return 0;
}

int SmoothieEngineInitInfo::destroy_device(VkDevice& device)
{
    vkDestroyDevice(device, nullptr);
    device = nullptr;
    return 0;
}

int SmoothieEngineInitInfo::getGraphicsQueue(VkQueue& queue) const
{
    vkGetDeviceQueue(SmoothieCore::getDevice(), queueFamilyGraphicsIndex.value(), 0, &queue);
    return 0;
}

int SmoothieEngineInitInfo::getGraphicsQueueFamilyIndex(unsigned int& queueFamilyIndex) const
{
    queueFamilyIndex = queueFamilyGraphicsIndex.value();
    return 0;
}

int SmoothieEngineInitInfo::getPresentQueue(VkQueue& queue) const
{
    vkGetDeviceQueue(SmoothieCore::getDevice(), queueFamilyPresentIndex.value(), 0, &queue);
    return 0;
}

int SmoothieEngineInitInfo::getPresentQueueFamilyIndex(unsigned int& queueFamilyIndex) const
{
    queueFamilyIndex = queueFamilyPresentIndex.value();
    return 0;
}

int SmoothieEngineInitInfo::getComputeQueue(VkQueue& queue) const
{
    return 0;
}

int SmoothieEngineInitInfo::getComputeQueueFamilyIndex(unsigned int& queueFamilyIndex) const
{
    return 0;
}

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

int SmoothieEngineInitInfo::create_swapchain(VkSwapchainKHR& swapchain)
{
    //Get surface capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(SmoothieCore::getPhysicalDevice(), SmoothieCore::getSurface(), &capabilities);

    //Get surface format
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(SmoothieCore::getPhysicalDevice(), SmoothieCore::getSurface(), &formatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
    if (formatCount != 0)
    {
        formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(SmoothieCore::getPhysicalDevice(), SmoothieCore::getSurface(), &formatCount, formats.data());
    }

    //Get present mode
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(SmoothieCore::getPhysicalDevice(), SmoothieCore::getSurface(), &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(SmoothieCore::getPhysicalDevice(), SmoothieCore::getSurface(), &presentModeCount, presentModes.data());
    }

    //Swap chain
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(SmoothieCore::getPhysicalDevice(), SmoothieCore::getSurface());

    surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent{};
    extent.width = SmoothieCore::getScrWidth();
    extent.height = SmoothieCore::getScrHeight();

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = SmoothieCore::getSurface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    unsigned int graphicsIndex = SmoothieCore::getQueueFamilyGraphicsIndex();
    unsigned int presentIndex = SmoothieCore::getQueueFamilyPresentIndex();
    uint32_t queueFamilyIndices[] = { graphicsIndex, presentIndex };

    if (graphicsIndex != presentIndex)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(SmoothieCore::getDevice(), &createInfo, nullptr, &swapchain) != VK_SUCCESS)
    {
        std::cout << "Failed to create swapchain!" << std::endl;
        return 1;
    }

    return 0;
}

int SmoothieEngineInitInfo::destroy_swapchain(VkSwapchainKHR& swapchain)
{
    vkDestroySwapchainKHR(SmoothieCore::getDevice(), swapchain, nullptr);
    swapchain = nullptr;
    return 0;
}

int SmoothieEngineInitInfo::get_swapchain_images(std::vector<VkImage>& images)
{
    images.resize(0);
    unsigned int imageCount = 0;
    if (vkGetSwapchainImagesKHR(SmoothieCore::getDevice(), SmoothieCore::getSwapchain(), &imageCount, nullptr) != VK_SUCCESS)
    {
        std::cout << "Can't get swapchain images!" << std::endl;
        return 1;
    }
    images.resize(imageCount);
    if (vkGetSwapchainImagesKHR(SmoothieCore::getDevice(), SmoothieCore::getSwapchain(), &imageCount, images.data()) != VK_SUCCESS)
    {
        std::cout << "Can't get swapchain images!" << std::endl;
        return 1;
    }
    return 0;
}

int SmoothieEngineInitInfo::create_swapchain_image_views(std::vector<VkImageView>& imageViews)
{
    imageViews.resize(SmoothieCore::getSwapchainImages().size());
    for (size_t i = 0; i < imageViews.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = SmoothieCore::getSwapchainImage(i);
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = surfaceFormat.format;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(SmoothieCore::getDevice(), &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS)
        {
            std::cout << "Failed to create swapchain image view!" << std::endl;
            return 1;
        }
    }

    return 0;
}

int SmoothieEngineInitInfo::destroy_swapchain_image_views(std::vector<VkImageView>& imageViews)
{
    for (size_t i = 0; i < imageViews.size(); i++) 
    {
        vkDestroyImageView(SmoothieCore::getDevice(), imageViews[i], nullptr);
        imageViews[i] = nullptr;
    }
    return 0;
}

int SmoothieEngineInitInfo::create_swapchain_framebuffers(std::vector<VkFramebuffer>& framebuffers)
{
    framebuffers.resize(SmoothieCore::getSwapchainImageViews().size());
    for (size_t i = 0; i < framebuffers.size(); i++)
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = SmoothieCore::getDefaultRenderPass();
        framebufferInfo.attachmentCount = 1;
        const VkImageView imageView = SmoothieCore::getSwapchainImageView(i);
        framebufferInfo.pAttachments = &imageView;
		framebufferInfo.width = SmoothieCore::getScrWidth();
		framebufferInfo.height = SmoothieCore::getScrHeight();
		framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(SmoothieCore::getDevice(), &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
        {
            std::cout << "Failed to create framebuffer: " << i << " Can't continue with initialization!" << std::endl;
            return 1;
        }
	}

    return 0;
}

void SmoothieEngineInitInfo::destroy_swapchain_framebuffers(std::vector<VkFramebuffer>& framebuffers)
{
    for (auto& framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr);
        framebuffer = nullptr;
    }
}

int SmoothieEngineInitInfo::create_default_renderpass(VkRenderPass& renderPass)
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = surfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    if (vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        std::cout << "Failed to create default render pass!" << std::endl;
        return 1;
    }
    return 0;
}

void SmoothieEngineInitInfo::destroy_default_renderpass(VkRenderPass& renderPass)
{
    vkDestroyRenderPass(SmoothieCore::getDevice(), renderPass, nullptr);
    renderPass = nullptr;
}
