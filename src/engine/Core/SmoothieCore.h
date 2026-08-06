#pragma once
#include "../Common.h"
#include "Camera.h"
#include "Resource.h"
#include "Texture.h"
#include "Buffer.h"
#include "Drawing.h"


namespace Smoothie 
{
    struct EngineInitInfo
    {
        VkInstance instance = nullptr;
        VkSurfaceKHR surface = nullptr;
        VkPhysicalDevice physical_device = nullptr;
        VkDevice device = nullptr;

        unsigned int queue_family_index_graphics = -1;
        VkQueue queue_graphics = nullptr;

        unsigned int queue_family_index_present = -1;
        VkQueue queue_present = nullptr;

        unsigned int queue_family_index_worker = -1;
        VkQueue queue_worker = nullptr;

        unsigned int swapchain_image_count = 0;
        VkSwapchainKHR swapchain = nullptr;
        VkFormat swapchain_image_format = VK_FORMAT_UNDEFINED;

        unsigned int width = 1280;
        unsigned int height = 720;
        unsigned int frames_in_flight = 3;

        std::shared_ptr<Drawing_Base> draw_class = nullptr;
    };

    struct Debug_Info
    {
        enum Message_Severity
        {
            Message_Severity_VERBOSE,
            Message_Severity_INFO,
            Message_Severity_WARNING,
            Message_Severity_ERROR,
            Message_Severity_CRITICAL,
            Message_Severity_PERFORMANCE,
        };
        Message_Severity severity;
        const std::string& message;
        std::thread::id thread_id;

        explicit Debug_Info(Message_Severity severity, const std::string& msg): severity(severity), message(msg), thread_id(std::this_thread::get_id()) {}
    };

    typedef void (*DebugCallbackFunction)(const Debug_Info& info);

}


class SmoothieCore
{
public:
    static int initEngine(const Smoothie::EngineInitInfo& initInfo);
    static int finitEngine();
    static void draw();

    static int loadScene(const std::string& scene_file);
    static int swap_scene(const std::string& scene_file);
    static void removeScene();

    static void getRequiredPhysicalDeviceFeatures(VkPhysicalDeviceFeatures& features);
    static void getRequiredPhysicalDeviceFeatures_Vk11(VkPhysicalDeviceVulkan11Features& features);
    static void getRequiredPhysicalDeviceFeatures_Vk12(VkPhysicalDeviceVulkan12Features& features);
    static void getRequiredPhysicalDeviceFeatures_Vk13(VkPhysicalDeviceVulkan13Features& features);
    static void getRequiredPhysicalDeviceFeatures_Vk14(VkPhysicalDeviceVulkan14Features& features);
    static void getRequiredDeviceExtensions(std::vector<const char*>& extensions);
    static constexpr uint32_t getRequiredVulkanAPIVersion() {return VK_API_VERSION_1_3;}

    //Logging

    static void setLoggingCallback(Smoothie::DebugCallbackFunction function);

    static void logVerbose(const std::string& msg) noexcept; //Thread safe.
    static void logInfo(const std::string& msg) noexcept; //Thread safe.
    static void logWarning(const std::string& msg) noexcept; //Thread safe.
    static void logError(const std::string& msg) noexcept; //Thread safe.
    static void logCritical(const std::string& msg) noexcept; //Thread safe.
    static void logPerformance(const std::string& msg) noexcept; //Thread safe.

    template <typename... T> static void logVerbose_Formated(fmt::format_string<T...> fmt, T&&... args) {logVerbose(fmt::format(fmt, std::forward<T>(args)...));}
    template <typename... T> static void logInfo_Formated(fmt::format_string<T...> fmt, T&&... args) {logInfo(fmt::format(fmt, std::forward<T>(args)...));}
    template <typename... T> static void logWarning_Formated(fmt::format_string<T...> fmt, T&&... args) {logWarning(fmt::format(fmt, std::forward<T>(args)...));}
    template <typename... T> static void logError_Formated(fmt::format_string<T...> fmt, T&&... args) {logError(fmt::format(fmt, std::forward<T>(args)...));}
    template <typename... T> static void logCritical_Formated(fmt::format_string<T...> fmt, T&&... args) {logCritical(fmt::format(fmt, std::forward<T>(args)...));}
    template <typename... T> static void logPerformance_Formated(fmt::format_string<T...> fmt, T&&... args) {logPerformance(fmt::format(fmt, std::forward<T>(args)...));}

    //Resources

    template<class ResourceType> static void Resource_SubmitAndLoad(const std::string& filepath)
    {
        static_assert(std::is_base_of_v<Smoothie::Resource_Base, ResourceType>);
        SubmitAndLoad(filepath, std::make_shared<ResourceType>());
    }

    //template<class ResourceType> Resource_WaitAndGet(const std::string& filepath){}





    //Updates render engine with new camera data.
    static void updateCameraData(const Smoothie::Camera& camera);

    //Old swapchain gets automatically destroyed.
    static void updateRenderingResolution(unsigned int windowWidth, unsigned int windowHeight, VkSwapchainKHR new_swapchain);

    static VkInstance getInstance() { return s_Instance; };
    static VkPhysicalDevice getPhysicalDevice() { return s_PhysicalDevice; };
    static VkDevice getDevice() { return s_Device; };
    static VkSurfaceKHR getSurface() { return s_Surface; };
    static VkQueue getGraphicsQueue() { return s_Queue_Graphics; }
    static VkQueue getPresentQueue() { return s_Queue_Present; }
    static VkCommandPool getMainCommandPool() { return s_CommandPool; }
    static VkSwapchainKHR getSwapchain() { return s_Swapchain; }
    static VkImage getSwapchainImage(unsigned int index) { return s_Swapchain_Images[index]; }
    static VkImageView getSwapchainImageView(unsigned int index) { return s_Swapchain_ImageViews[index]; }
    static VkRenderPass getDefaultRenderPass() { return s_Default_RenderPass; }
    static VkFramebuffer getSwapchainFramebuffer(unsigned int index) { return s_Swapchain_Framebuffers[index]; }
    static VmaAllocator getVulkanMemoryAllocator() { return s_VmaAllocator; }
    static VkFormat getSwapchainFormat() {return s_Default_Format;}
    static const std::vector<VkImage>& getSwapchainImages() { return s_Swapchain_Images; }
    static const std::vector<VkImageView>& getSwapchainImageViews() { return s_Swapchain_ImageViews; }
    static const std::vector<VkFramebuffer>& getSwapchainFramebuffers() { return s_Swapchain_Framebuffers; }

    static unsigned int getQueueFamilyGraphicsIndex() { return s_QueueFamilyIndex_Graphics; }
    static unsigned int getQueueFamilyPresentIndex() { return s_QueueFamilyIndex_Present; }
    static unsigned int getScrWidth() {return s_WindowWidth;}
    static unsigned int getScrHeight() {return s_WindowHeight;}
    static unsigned int getCurrentFrame() { return s_CurrentFrameInFlight; }
    static unsigned int getFramesInFlightCount() {return s_FramesInFlightCount;}

    static std::shared_ptr<Smoothie::Drawing_Base>& getDrawingClassPtr() {return s_DrawClass; }

    static Smoothie::DefaultTexture2D getDefault2DTexture() { return s_Default_Texture; }
    static Smoothie::DefaultBuffer getDefaultBuffer() { return s_Default_Buffer; }
    static std::mutex& getQueueSubmitMutex() {return s_WorkQueueMutex;}

    static void setViewport(VkCommandBuffer commandBuffer)
    {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(s_WindowWidth);
        viewport.height = static_cast<float>(s_WindowHeight);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    };
    static void setScissor(VkCommandBuffer commandBuffer)
    {
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent.width = s_WindowWidth;
        scissor.extent.height = s_WindowHeight;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    SmoothieCore(const SmoothieCore&) = delete;
private:
    static VkInstance s_Instance;
    static VkSurfaceKHR s_Surface;
    static VkPhysicalDevice s_PhysicalDevice;
    static VkDevice s_Device;

    static unsigned int s_QueueFamilyIndex_Graphics;
    static VkQueue s_Queue_Graphics;
    static unsigned int s_QueueFamilyIndex_Present;
    static VkQueue s_Queue_Present;
    static unsigned int s_QueueFamilyIndex_Work;
    static VkQueue s_Queue_Work;

    static VkSwapchainKHR s_Swapchain;
    static unsigned int s_WindowWidth, s_WindowHeight;

    static VkFormat s_Default_Format;
    static VkRenderPass s_Default_RenderPass;
    static std::vector<VkImage> s_Swapchain_Images;
    static std::vector<VkImageView> s_Swapchain_ImageViews;
    static std::vector<VkFramebuffer> s_Swapchain_Framebuffers;

    static VkCommandPool s_CommandPool;
    static VmaAllocator s_VmaAllocator;

    static Smoothie::DefaultTexture2D s_Default_Texture;
    static Smoothie::DefaultBuffer s_Default_Buffer;

    static unsigned int s_CurrentFrameInFlight;
    static unsigned int s_FramesInFlightCount;
    static std::mutex s_WorkQueueMutex;

    static std::shared_ptr<Smoothie::Drawing_Base> s_DrawClass;

    static std::mutex s_Resources_Mutex;
    static std::unordered_map<std::string, std::shared_ptr<Smoothie::internal::ResourceEntry>> s_Resources;
    static std::unique_ptr<Smoothie::internal::ResourceThreadPool> s_ResourceThreadPool;
    static void SubmitAndLoad(const std::string &filepath, std::shared_ptr<Smoothie::Resource_Base> type);

    SmoothieCore() = default;
};