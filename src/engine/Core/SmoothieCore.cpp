#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "SmoothieCore.h"

using namespace Smoothie;

VkInstance SmoothieCore::s_Instance = nullptr;
VkSurfaceKHR SmoothieCore::s_Surface = nullptr;
VkPhysicalDevice SmoothieCore::s_PhysicalDevice = nullptr;
VkDevice SmoothieCore::s_Device = nullptr;

unsigned int SmoothieCore::s_QueueFamilyIndex_Graphics = -1;
VkQueue SmoothieCore::s_Queue_Graphics = nullptr;
unsigned int SmoothieCore::s_QueueFamilyIndex_Present = -1;
VkQueue SmoothieCore::s_Queue_Present = nullptr;
unsigned int SmoothieCore::s_QueueFamilyIndex_Work = -1;
VkQueue SmoothieCore::s_Queue_Work = nullptr;

VkSwapchainKHR SmoothieCore::s_Swapchain = nullptr;
unsigned int SmoothieCore::s_WindowWidth = 0;
unsigned int SmoothieCore::s_WindowHeight = 0;

VkFormat SmoothieCore::s_Default_Format = VK_FORMAT_UNDEFINED;
VkRenderPass SmoothieCore::s_Default_RenderPass = nullptr;
std::vector<VkImage> SmoothieCore::s_Swapchain_Images;
std::vector<VkImageView> SmoothieCore::s_Swapchain_ImageViews;
std::vector<VkFramebuffer> SmoothieCore::s_Swapchain_Framebuffers;

VkCommandPool SmoothieCore::s_CommandPool = nullptr;
VmaAllocator SmoothieCore::s_VmaAllocator = nullptr;

DefaultTexture2D SmoothieCore::s_Default_Texture;
DefaultBuffer SmoothieCore::s_Default_Buffer;

unsigned int SmoothieCore::s_CurrentFrameInFlight = 0;
unsigned int SmoothieCore::s_FramesInFlightCount = 0;
std::mutex SmoothieCore::s_WorkQueueMutex;

std::shared_ptr<Drawing_Base> SmoothieCore::s_DrawClass = nullptr;

static std::vector<VkCommandBuffer> g_RenderCommandBuffers;
static std::vector<VkSemaphore> g_ImageAvailableSemaphores;
static std::vector<VkSemaphore> g_RenderFinishedSemaphores;
static std::vector<VkFence> g_InFlightFences;
static bool g_IsEngineReady = false;

std::unordered_map<std::string, std::shared_ptr<internal::ResourceEntry>> SmoothieCore::s_Resources;
std::mutex SmoothieCore::s_Resources_Mutex;
std::unique_ptr<internal::ResourceThreadPool> SmoothieCore::s_ResourceThreadPool;

int create_default_render_pass(VkDevice device, VkRenderPass &render_pass, VkFormat format)
{
    assert(device != nullptr);

    VkAttachmentDescription _attachment_description{};
    _attachment_description.format = format;
    _attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    _attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    _attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    _attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    _attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    _attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference _attachment_ref{};
    _attachment_ref.attachment = 0;
    _attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &_attachment_ref;

    VkRenderPassCreateInfo _render_pass_create_info{};
    _render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    _render_pass_create_info.attachmentCount = 1;
    _render_pass_create_info.pAttachments = &_attachment_description;
    _render_pass_create_info.subpassCount = 1;
    _render_pass_create_info.pSubpasses = &subpass;
    _render_pass_create_info.dependencyCount = 0;
    _render_pass_create_info.pDependencies = nullptr;
    if (vkCreateRenderPass(device, &_render_pass_create_info, nullptr, &render_pass) != VK_SUCCESS)
    {
        SmoothieCore::logError("Failed to create render pass!");
        return 1;
    }
    return 0;
}

static int create_swpachain_image_data(std::vector<VkImage> &s_Swapchain_Images,
                                       std::vector<VkImageView> &s_Swapchain_ImageViews, VkFormat format)
{
    unsigned int _swapchain_img_count = 0;
    if (vkGetSwapchainImagesKHR(SmoothieCore::getDevice(), SmoothieCore::getSwapchain(), &_swapchain_img_count, nullptr)
        != VK_SUCCESS)
    {
        SmoothieCore::logError("Failed to get swapchain images!");
        return 1;
    }
    s_Swapchain_Images.resize(_swapchain_img_count);
    if (vkGetSwapchainImagesKHR(SmoothieCore::getDevice(), SmoothieCore::getSwapchain(), &_swapchain_img_count,
                                s_Swapchain_Images.data()) != VK_SUCCESS)
    {
        SmoothieCore::logError("Failed to get swapchain images!");
        return 1;
    }

    s_Swapchain_ImageViews.resize(_swapchain_img_count);
    for (size_t i = 0; i < s_Swapchain_ImageViews.size(); i++)
    {
        VkImageViewCreateInfo _image_view_create_info{};
        _image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        _image_view_create_info.image = SmoothieCore::getSwapchainImage(i);
        _image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        _image_view_create_info.format = format;
        _image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        _image_view_create_info.subresourceRange.baseMipLevel = 0;
        _image_view_create_info.subresourceRange.levelCount = 1;
        _image_view_create_info.subresourceRange.baseArrayLayer = 0;
        _image_view_create_info.subresourceRange.layerCount = 1;
        if (vkCreateImageView(SmoothieCore::getDevice(), &_image_view_create_info, nullptr,
                              &s_Swapchain_ImageViews[i]) != VK_SUCCESS)
        {
            SmoothieCore::logError("Failed to create swapchain image view: " + std::to_string(i));
            return 1;
        }
    }

    return 0;
}

static void destroy_swpachain_image_data(std::vector<VkImage> &s_Swapchain_Images,
                                         std::vector<VkImageView> &s_Swapchain_ImageViews)
{
    s_Swapchain_Images = std::vector<VkImage>();
    for (auto &_view: s_Swapchain_ImageViews)
    {
        if (_view != nullptr) vkDestroyImageView(SmoothieCore::getDevice(), _view, nullptr), _view = nullptr;
    }
}

static int create_swapchain_framebuffers(
    std::vector<VkFramebuffer> &framebuffers,
    const std::vector<VkImageView> &imageViews,
    VkRenderPass renderPass,
    unsigned int width,
    unsigned int height)
{
    framebuffers.resize(imageViews.size());
    for (size_t i = 0; i < framebuffers.size(); i++)
    {
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &imageViews[i];
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(SmoothieCore::getDevice(), &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
        {
            SmoothieCore::logError("Failed to create swapchain framebuffer: " + std::to_string(i));
            return 1;
        }
    }

    return 0;
}

static void destroy_swapchain_framebuffers(std::vector<VkFramebuffer> &framebuffers)
{
    for (auto &_framebuffer: framebuffers)
    {
        if (_framebuffer == nullptr) continue;

        vkDestroyFramebuffer(SmoothieCore::getDevice(), _framebuffer, nullptr), _framebuffer = nullptr;
        _framebuffer = nullptr;
    }
}

int SmoothieCore::initEngine(const Smoothie::EngineInitInfo &initInfo)
{
    s_Instance = initInfo.instance;
    assert(s_Instance != nullptr);
    if (s_Instance == nullptr)
    {
        logError("VkInstance must be provided!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Selected VkInstance: {}", fmt::ptr(s_Instance));

    s_Surface = initInfo.surface;
    assert(s_Surface != nullptr);
    if (s_Surface == nullptr)
    {
        logError("VkSurfaceKHR must be provided!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Selected VkSurfaceKHR: {}", fmt::ptr(s_Surface));


    s_PhysicalDevice = initInfo.physical_device;
    assert(s_PhysicalDevice != nullptr);
    if (s_PhysicalDevice == nullptr)
    {
        logError("VkPhysicalDevice must be provided!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Selected VkPhysicalDevice: {}", fmt::ptr(s_PhysicalDevice));

    s_Device = initInfo.device;
    assert(s_Device != nullptr);
    if (s_Device == nullptr)
    {
        logError("VkDevice must be provided!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Selected VkDevice: {}", fmt::ptr(s_Device));

    s_QueueFamilyIndex_Graphics = initInfo.queue_family_index_graphics;
    if (s_QueueFamilyIndex_Graphics == -1)
    {
        logWarning_Formated(
            "Queue family index for graphics is: {}; and that is default initialized value; it might be wrong.",
            static_cast<unsigned int>(-1));
    }

    s_Queue_Graphics = initInfo.queue_graphics;
    assert(s_Queue_Graphics != nullptr);
    if (s_Queue_Graphics == nullptr)
    {
        logError("VkQueue for graphics is not provided!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Selected VkQueue for graphics: {}; Family Index: {}", fmt::ptr(s_Queue_Graphics),
                        s_QueueFamilyIndex_Graphics);

    s_QueueFamilyIndex_Present = initInfo.queue_family_index_present;
    if (s_QueueFamilyIndex_Present == -1)
    {
        logWarning(
            "Queue family index for present is: " + std::to_string(static_cast<unsigned int>(-1)) +
            " and that is default initialized value; it might be wrong!");
    }

    s_Queue_Present = initInfo.queue_present;
    assert(s_Queue_Present != nullptr);
    if (s_Queue_Present == nullptr)
    {
        logError("VkQueue for present is not provided!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Selected VkQueue for presentation: {}; Family Index: {}", fmt::ptr(s_Queue_Present),
                        s_QueueFamilyIndex_Present);

    //Work queue is not yet implemented

    s_Swapchain = initInfo.swapchain;
    assert(s_Swapchain != nullptr);
    if (s_Swapchain == nullptr)
    {
        logError("VkSwapchainKHR must be provided!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Selected VkSwapchainKHR: {}", fmt::ptr(s_Swapchain));

    s_WindowWidth = initInfo.width;
    if (s_WindowWidth == 0)
    {
        logError("Window with must be provided!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Drawing width: {}", s_WindowWidth);

    s_WindowHeight = initInfo.height;
    if (s_WindowHeight == 0)
    {
        logError("Window height must be provided!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Drawing height: {}", s_WindowHeight);

    s_Default_Format = initInfo.swapchain_image_format;
    if (s_Default_Format == VK_FORMAT_UNDEFINED)
    {
        logWarning("VkFormat must be a valid format!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Surface format: {}", static_cast<unsigned int>(s_Default_Format));

    if (create_default_render_pass(s_Device, s_Default_RenderPass, s_Default_Format) != 0)
    {
        logError("Failed to create default render pass!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Created default render VkRenderPass: {} ", fmt::ptr(s_Default_RenderPass));

    if (create_swpachain_image_data(s_Swapchain_Images, s_Swapchain_ImageViews, s_Default_Format) != 0)
    {
        logError("Failed to create swapchain image data!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Acquired {} VkImage from VkSwapchainKHR: {}", s_Swapchain_Images.size(),
                        fmt::join(s_Swapchain_Images, ", "));
    logVerbose_Formated("Created {} VkImageView for swapchain images: {}", s_Swapchain_ImageViews.size(),
                        fmt::join(s_Swapchain_ImageViews, ", "));

    if (create_swapchain_framebuffers(s_Swapchain_Framebuffers, s_Swapchain_ImageViews, s_Default_RenderPass,
                                      s_WindowWidth, s_WindowHeight) != 0)
    {
        logError("Failed to create swapchain framebuffers!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Created {} VkFramebuffer: {}", s_Swapchain_Framebuffers.size(),
                        fmt::join(s_Swapchain_Framebuffers, ", "));

    VkCommandPoolCreateInfo _pool_create_info{};
    _pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    _pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    _pool_create_info.queueFamilyIndex = s_QueueFamilyIndex_Graphics;
    if (vkCreateCommandPool(s_Device, &_pool_create_info, nullptr, &s_CommandPool) != VK_SUCCESS)
    {
        logError("Failed to create main command pool!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Created main VkCommandPool: {}", fmt::ptr(s_CommandPool));

    VmaVulkanFunctions _vk_functions = {};
    _vk_functions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    _vk_functions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo _allocator_create_info = {};
    _allocator_create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    _allocator_create_info.vulkanApiVersion = getRequiredVulkanAPIVersion();
    _allocator_create_info.physicalDevice = s_PhysicalDevice;
    _allocator_create_info.device = s_Device;
    _allocator_create_info.instance = s_Instance;
    _allocator_create_info.pVulkanFunctions = &_vk_functions;
    if (vmaCreateAllocator(&_allocator_create_info, &s_VmaAllocator) != VK_SUCCESS)
    {
        logError("Failed to create VMA Allocator!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Created VmaAllocator: {}", fmt::ptr(s_VmaAllocator));


    if (s_Default_Texture.create() != 0)
    {
        logError("Failed to create default 2D Texture!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Created default 2D Texture: VkImage: {}, VkImageView: {}, VmaAllocation: {}",
                        fmt::ptr(s_Default_Texture.getImage()), fmt::ptr(s_Default_Texture.getImageView()),
                        fmt::ptr(s_Default_Texture.getAllocation()));

    if (s_Default_Buffer.create() != 0)
    {
        logError("Failed to create default buffer!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Created default buffer: VkBuffer: {}, VmaAllocation: {}",
                        fmt::ptr(s_Default_Buffer.getBuffer()), fmt::ptr(s_Default_Buffer.getAllocation()));

    s_FramesInFlightCount = initInfo.frames_in_flight;
    if (s_FramesInFlightCount == 0)
    {
        logError("Failed to provide number of frames in flight!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Number of flames in-flight: {}", s_FramesInFlightCount);

    VkCommandBufferAllocateInfo _command_buffer_allocate_info{};
    _command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    _command_buffer_allocate_info.commandPool = s_CommandPool;
    _command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    _command_buffer_allocate_info.commandBufferCount = 1;
    g_RenderCommandBuffers.resize(s_FramesInFlightCount);

    VkSemaphoreCreateInfo _semaphore_create_info{};
    _semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    g_ImageAvailableSemaphores.resize(s_FramesInFlightCount);

    VkFenceCreateInfo _fence_create_info{};
    _fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    _fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    g_InFlightFences.resize(s_FramesInFlightCount);
    for (unsigned int i = 0; i < s_FramesInFlightCount; i++)
    {
        if (vkAllocateCommandBuffers(s_Device, &_command_buffer_allocate_info, &g_RenderCommandBuffers[i]) !=
            VK_SUCCESS)
        {
            logError("Failed to allocate main command buffer: " + std::to_string(i));
            finitEngine();
            return 1;
        }

        if (vkCreateSemaphore(s_Device, &_semaphore_create_info, nullptr, &g_ImageAvailableSemaphores[i]) != VK_SUCCESS)
        {
            logError("Failed to allocate image available semaphore: " + std::to_string(i));
            finitEngine();
            return 1;
        }

        if (vkCreateFence(s_Device, &_fence_create_info, nullptr, &g_InFlightFences[i]) != VK_SUCCESS)
        {
            logError("Failed to allocate fence: " + std::to_string(i));
            finitEngine();
            return 1;
        }
    }

    g_RenderFinishedSemaphores.resize(s_Swapchain_Images.size());
    for (unsigned int i = 0; i < s_Swapchain_Images.size(); i++)
    {
        if (vkCreateSemaphore(s_Device, &_semaphore_create_info, nullptr, &g_RenderFinishedSemaphores[i]) != VK_SUCCESS)
        {
            logError("Failed to create render finished semaphore: " + std::to_string(i));
            finitEngine();
            return 1;
        }
    }

    logVerbose_Formated("Allocated main command buffers: {}", fmt::join(g_RenderCommandBuffers, ", "));
    logVerbose_Formated("Created image available semaphores: {}", fmt::join(g_ImageAvailableSemaphores, ", "));
    logVerbose_Formated("Created fences: {}", fmt::join(g_InFlightFences, ", "));
    logVerbose_Formated("Created render finished semaphores: {}", fmt::join(g_RenderFinishedSemaphores, ", "));

    logInfo("Engine initialized.");
    s_DrawClass = initInfo.draw_class;
    assert(s_DrawClass != nullptr);
    if (s_DrawClass == nullptr)
    {
        logError("Draw class must be provided!");
        finitEngine();
        return 1;
    }
    logVerbose_Formated("Selected draw class: {}", fmt::ptr(s_DrawClass));

    if (s_DrawClass->create() != 0)
    {
        logError("Failed to create drawing class!");
        finitEngine();
        return 1;
    }


    g_IsEngineReady = true;
    return 0;
}

int SmoothieCore::finitEngine()
{
    removeScene();
    vkDeviceWaitIdle(s_Device);

    if (s_DrawClass != nullptr)
    {
        s_DrawClass->destroy();
    }

    for (unsigned int i = 0; i < s_FramesInFlightCount; i++)
    {
        if (g_InFlightFences[i] != nullptr)
        {
            vkDestroyFence(s_Device, g_InFlightFences[i], nullptr);
            g_InFlightFences[i] = nullptr;
        }

        if (g_ImageAvailableSemaphores[i] != nullptr)
        {
            vkDestroySemaphore(s_Device, g_ImageAvailableSemaphores[i], nullptr);
            g_ImageAvailableSemaphores[i] = nullptr;
        }
        g_RenderCommandBuffers[i] = nullptr;
    }
    logVerbose("Destroyed image available semaphores");


    for (unsigned int i = 0; i < s_Swapchain_Images.size(); i++)
    {
        vkDestroySemaphore(s_Device, g_RenderFinishedSemaphores[i], nullptr);
        g_RenderFinishedSemaphores[i] = nullptr;
    }
    logVerbose("Destroyed in-flight fences and render finished semaphores.");

    s_FramesInFlightCount = 0;

    s_Default_Buffer.destroy();
    logVerbose("Destroyed default buffer.");
    s_Default_Texture.destroy();
    logVerbose("Destroyed default texture.");

    /*char* statsStr = nullptr;
    vmaBuildStatsString(SmoothieCore::getVulkanMemoryAllocator(), &statsStr, VK_TRUE);
    std::cout << statsStr << std::endl;
    vmaFreeStatsString(SmoothieCore::getVulkanMemoryAllocator(), statsStr);*/

    if (s_VmaAllocator != nullptr)
    {
        vmaDestroyAllocator(s_VmaAllocator);
        logVerbose("Destroyed VmaAllocator.");
        s_VmaAllocator = nullptr;
    }

    if (s_CommandPool != nullptr)
    {
        vkDestroyCommandPool(s_Device, s_CommandPool, nullptr);
        logVerbose("Destroyed main VkCommandPool.");
        s_CommandPool = nullptr;
    }

    destroy_swapchain_framebuffers(s_Swapchain_Framebuffers);
    logVerbose("Destroyed framebuffers.");

    destroy_swpachain_image_data(s_Swapchain_Images, s_Swapchain_ImageViews);
    logVerbose("Destroyed swapchain image views.");

    if (s_Default_RenderPass != nullptr)
    {
        vkDestroyRenderPass(s_Device, s_Default_RenderPass, nullptr);
        s_Default_RenderPass = nullptr;
        logVerbose("Destroyed default render pass.");
    }

    s_Swapchain = nullptr;
    s_Device = nullptr;
    s_Surface = nullptr;
    s_Instance = nullptr;
    logInfo("Engine un-initialized.");
    return 0;
}

int SmoothieCore::loadScene(const std::string &scene_file)
{
    // auto _scene = s_Scene.get();
    // if ((_scene == nullptr) || (g_IsEngineReady == false))
    // {
    // 	std::cout << "Can't load scene because engine is not initialized!" << std::endl;
    // 	return 1;
    // }
    //
    // if (isSceneLoaded == true)
    // {
    // 	std::cout << "Scene is already loaded!" << std::endl;
    // 	return 1;
    // }
    //
    // const int err = _scene->load_scene(scene_file);
    //
    // return err;
    return 0;
}

int SmoothieCore::swap_scene(const std::string &scene_file)
{
    return 0;
}

void SmoothieCore::removeScene()
{
    // auto _scene = s_Scene.get();
    // if ((_scene == nullptr) || (g_IsEngineReady == false)) return;
    //
    // _scene->_clear_scene();
}

void SmoothieCore::getRequiredPhysicalDeviceFeatures(VkPhysicalDeviceFeatures &features) {}

void SmoothieCore::getRequiredPhysicalDeviceFeatures_Vk11(VkPhysicalDeviceVulkan11Features &features)
{
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    features.pNext = nullptr;
}

void SmoothieCore::getRequiredPhysicalDeviceFeatures_Vk12(VkPhysicalDeviceVulkan12Features &features)
{
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features.pNext = nullptr;
}

void SmoothieCore::getRequiredPhysicalDeviceFeatures_Vk13(VkPhysicalDeviceVulkan13Features &features)
{
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features.pNext = nullptr;
    features.dynamicRendering = true;
    features.synchronization2 = true;
}

void SmoothieCore::getRequiredPhysicalDeviceFeatures_Vk14(VkPhysicalDeviceVulkan14Features &features)
{
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
    features.pNext = nullptr;
}

void SmoothieCore::getRequiredDeviceExtensions(std::vector<const char *> &extensions)
{
    extensions.clear();
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

static DebugCallbackFunction g_CallbackFunction = nullptr;
void SmoothieCore::setLoggingCallback(DebugCallbackFunction function) { g_CallbackFunction = function; }

static std::mutex g_loggingMutex;

void SmoothieCore::logVerbose(const std::string &msg) noexcept
{
    if (g_CallbackFunction == nullptr) return;
    std::lock_guard<std::mutex> lock(g_loggingMutex);
    g_CallbackFunction(Debug_Info(Debug_Info::Message_Severity_VERBOSE, msg));
}

void SmoothieCore::logInfo(const std::string &msg) noexcept
{
    if (g_CallbackFunction == nullptr) return;
    std::lock_guard<std::mutex> lock(g_loggingMutex);
    g_CallbackFunction(Debug_Info(Debug_Info::Message_Severity_INFO, msg));
}

void SmoothieCore::logWarning(const std::string &msg) noexcept
{
    if (g_CallbackFunction == nullptr) return;
    std::lock_guard<std::mutex> lock(g_loggingMutex);
    g_CallbackFunction(Debug_Info(Debug_Info::Message_Severity_WARNING, msg));
}

void SmoothieCore::logError(const std::string &msg) noexcept
{
    if (g_CallbackFunction == nullptr) return;
    std::lock_guard<std::mutex> lock(g_loggingMutex);
    g_CallbackFunction(Debug_Info(Debug_Info::Message_Severity_ERROR, msg));
}

void SmoothieCore::logCritical(const std::string &msg) noexcept
{
    if (g_CallbackFunction == nullptr) return;
    std::lock_guard<std::mutex> lock(g_loggingMutex);
    g_CallbackFunction(Debug_Info(Debug_Info::Message_Severity_CRITICAL, msg));
}

void SmoothieCore::logPerformance(const std::string &msg) noexcept
{
    if (g_CallbackFunction == nullptr) return;
    std::lock_guard<std::mutex> lock(g_loggingMutex);
    g_CallbackFunction(Debug_Info(Debug_Info::Message_Severity_PERFORMANCE, msg));
}

void SmoothieCore::updateCameraData(const Smoothie::Camera &camera)
{
    if (s_DrawClass != nullptr)
    {
        s_DrawClass->update_camera(camera);
    }
}

void SmoothieCore::updateRenderingResolution(unsigned int windowWidth, unsigned int windowHeight, VkSwapchainKHR new_swapchain)
{
    vkDeviceWaitIdle(s_Device);
    if (windowWidth < 2 || windowHeight < 2)
    {
        g_IsEngineReady = false;
        return;
    }

    s_WindowWidth = windowWidth;
    s_WindowHeight = windowHeight;
    g_IsEngineReady = true;

    destroy_swapchain_framebuffers(s_Swapchain_Framebuffers);
    destroy_swpachain_image_data(s_Swapchain_Images, s_Swapchain_ImageViews);

    if (s_Swapchain != nullptr)
    {
        vkDestroySwapchainKHR(s_Device, s_Swapchain, nullptr);
        s_Swapchain = new_swapchain;
    }

    if (create_swpachain_image_data(s_Swapchain_Images, s_Swapchain_ImageViews, s_Default_Format) != 0)
    {
        logError("Failed to re-create swapchain image data!");
        g_IsEngineReady = false;
        return;
    }

    if (create_swapchain_framebuffers(s_Swapchain_Framebuffers, s_Swapchain_ImageViews, s_Default_RenderPass,
                                      windowWidth, windowHeight) != 0)
    {
        logError("Failed to re-create swapchain framebuffers!");
        g_IsEngineReady = false;
        return;
    }

    if (s_DrawClass == nullptr) g_IsEngineReady = false;

    if (s_DrawClass->resize_callback() != 0)
    {
        logError("Failed to re-create draw class data!");
        g_IsEngineReady = false;
    }
}

void SmoothieCore::draw()
{
    if (g_IsEngineReady == false) return;

    vkWaitForFences(SmoothieCore::getDevice(), 1, &g_InFlightFences[s_CurrentFrameInFlight], VK_TRUE, UINT64_MAX);
    vkResetFences(SmoothieCore::getDevice(), 1, &g_InFlightFences[s_CurrentFrameInFlight]);
    uint32_t imageIndex = 0;
    vkAcquireNextImageKHR(SmoothieCore::getDevice(), getSwapchain(), UINT64_MAX,
                          g_ImageAvailableSemaphores[s_CurrentFrameInFlight], VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(g_RenderCommandBuffers[s_CurrentFrameInFlight], 0);

    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(g_RenderCommandBuffers[s_CurrentFrameInFlight], &commandBufferBeginInfo);
    if (s_DrawClass != nullptr) s_DrawClass->draw(g_RenderCommandBuffers[s_CurrentFrameInFlight], imageIndex);
    vkEndCommandBuffer(g_RenderCommandBuffers[s_CurrentFrameInFlight]);


    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &g_ImageAvailableSemaphores[s_CurrentFrameInFlight];
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &g_RenderCommandBuffers[s_CurrentFrameInFlight];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &g_RenderFinishedSemaphores[imageIndex];
    if (s_Queue_Graphics == s_Queue_Work)
    {
        std::lock_guard<std::mutex> _lock(s_WorkQueueMutex);
        vkQueueSubmit(s_Queue_Graphics, 1, &submitInfo, g_InFlightFences[s_CurrentFrameInFlight]);
    } else
    {
        vkQueueSubmit(s_Queue_Graphics, 1, &submitInfo, g_InFlightFences[s_CurrentFrameInFlight]);
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &g_RenderFinishedSemaphores[imageIndex];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &s_Swapchain;
    presentInfo.pImageIndices = &imageIndex;

    if (s_Queue_Present == s_Queue_Work)
    {
        std::lock_guard<std::mutex> _lock(s_WorkQueueMutex);
        vkQueuePresentKHR(s_Queue_Present, &presentInfo);
    } else
    {
        vkQueuePresentKHR(s_Queue_Present, &presentInfo);
    }

    s_CurrentFrameInFlight = (s_CurrentFrameInFlight + 1) % s_FramesInFlightCount;
}

void SmoothieCore::SubmitAndLoad(const std::string &filepath, std::shared_ptr<Resource_Base> type)
{
    if (!std::filesystem::exists(filepath))
    {
        logError_Formated(R"(File "{}" not found.)", filepath);
        return;
    }

    std::lock_guard<std::mutex> _lock(s_Resources_Mutex);

    //1. If object is loaded, no further actions needed.
    if (s_Resources.find(filepath) != s_Resources.end()) return;

    //2. Allocate new entry
    auto _new_entry = std::make_shared<internal::ResourceEntry>();

    //_new_entry->current_status = Scene_ResourceEntry::Initialized;
    _new_entry->resource = std::move(type);
    _new_entry->reader_count = 0;
    _new_entry->write_time = std::filesystem::last_write_time(filepath);

    //3. Add new entry to the map.
    s_Resources.insert({filepath, _new_entry});

    //4. Submit work to the queue and mark it as its currently updating.
    _new_entry->current_status.store(internal::ResourceEntry_Status_Updating);

    assert(s_ResourceThreadPool != nullptr);
    s_ResourceThreadPool->add_job(internal::ResourceThread_JobType_Load, filepath, _new_entry);
}