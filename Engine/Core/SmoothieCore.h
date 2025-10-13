#pragma once
#include <string>
#include <optional>
#include <unordered_map>
#include <memory>
#include <array>
#include <vector>

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#include "Core/Camera.h"
#include "Core/Scene.h"
#include "Core/RenderPass.h"
#include "Core/Constants.h"
#include "Core/Texture.h"
#include "Core/Buffer.h"
#include "Core/Pipeline.h"
#include "Core/Model.h"
#include "Core/Drawing.h"
#include "Core/CameraDescriptor.h"

#include "Effects/Deferred_Core.h"

namespace Smoothie 
{
    //Base class for initializing the engine
    class SmoothieCore_Initialization
    {

    public:
        virtual int create_instance(VkInstance& instance) = 0;
        virtual int destroy_instance(VkInstance& instance) = 0;

        virtual int create_surface(VkSurfaceKHR& surface) = 0;
        virtual int destroy_surface(VkSurfaceKHR& surface) = 0;

        virtual int select_physical_device(VkPhysicalDevice& physicalDevice) = 0;

        virtual int create_device(VkDevice& device) = 0;
        virtual int destroy_device(VkDevice& device) = 0;

        virtual int getGraphicsQueue(VkQueue& queue) const = 0;
        virtual int getGraphicsQueueFamilyIndex(unsigned int& queueFamilyIndex) const = 0;

        virtual int getPresentQueue(VkQueue& queue) const = 0;
        virtual int getPresentQueueFamilyIndex(unsigned int& queueFamilyIndex) const = 0;

        virtual int getComputeQueue(VkQueue& queue) const = 0;
        virtual int getComputeQueueFamilyIndex(unsigned int& queueFamilyIndex) const = 0;
        

        virtual int create_swapchain(VkSwapchainKHR& swapchain) = 0;
        virtual int destroy_swapchain(VkSwapchainKHR& swapchain) = 0;

        virtual int get_swapchain_images(std::vector<VkImage>& images) = 0;
        virtual int create_swapchain_image_views(std::vector<VkImageView>& imageViews) = 0;
        virtual int destroy_swapchain_image_views(std::vector<VkImageView>& imageViews) = 0;

        virtual int create_swapchain_framebuffers(std::vector<VkFramebuffer>& framebuffers) = 0;
        virtual void destroy_swapchain_framebuffers(std::vector<VkFramebuffer>& framebuffers) = 0;

        virtual int create_default_renderpass(VkRenderPass& renderPass) = 0;
        virtual void destroy_default_renderpass(VkRenderPass& renderPass) = 0;

    };
}


class SmoothieCore
{
public:
    static int initEngine(
        std::shared_ptr<Smoothie::SmoothieCore_Initialization>& initInfo,
        unsigned int windowWidth,
        unsigned windowHeight,

        std::shared_ptr<Smoothie::Scene_Default>& scene_loader = std::make_shared<Smoothie::Scene_Default>(),
        std::shared_ptr<Smoothie::Drawing_Base> drawingClass = std::make_shared<Smoothie::DeferredRendering::Drawing>()
    );
    static int finitEngine();
    static void draw();

    static int loadScene(const std::string& scene_file);
    static void removeScene();

    //Updates render engine with new camera data.
    static void updateCameraData(const Smoothie::Camera& camera);

    static void updateRenderingResolution(unsigned int windowWidth, unsigned int windowHeight);

    static inline VkInstance getInstance() { return instance; };
    static inline VkPhysicalDevice getPhysicalDevice() { return physicalDevice; };
    static inline VkDevice getDevice() { return device; };
    static inline VkSurfaceKHR getSurface() { return surface; };
    static inline VkQueue getGraphicsQueue() { return graphicsQueue; }
    static inline VkQueue getPresentQueue() { return presentQueue; }
    static inline VkCommandPool getMainCommandPool() { return commandPool; }
    static inline VkSwapchainKHR getSwapchain() { return swapchain; }
    static inline VkImage getSwapchainImage(unsigned int index) { return swapchainImages[index]; }
    static inline VkImageView getSwapchainImageView(unsigned int index) { return swapchainImageViews[index]; }
    static inline VkRenderPass getDefaultRenderPass() { return defaultRenderPass; }
    static inline VkFramebuffer getSwapchainFramebuffer(unsigned int index) { return swapchainFramebuffers[index]; }
    static inline VmaAllocator getVulkanMemoryAllocator() { return vmaAllocator; }

    static inline VkDescriptorSet getCameraDescriptorSet() { return cameraDescriptorSets[currentFrame].getDescriptrotSet(); }
    static inline VkDescriptorSetLayout getCameraDescriptorSetLayout(unsigned int index) { return cameraDescriptorSets[index].getDescriptrotSetLayout(); }
    
    static inline std::vector<VkImage> getSwapchainImages() { return swapchainImages; }
    static inline std::vector<VkImageView> getSwapchainImageViews() { return swapchainImageViews; }
    static inline std::vector<VkFramebuffer> getSwapchainFramebuffers() { return swapchainFramebuffers; }

    static inline unsigned int getQueueFamilyGraphicsIndex() { return queueFamilyGraphicsIndex; }
    static inline unsigned int getQueueFamilyPresentIndex() { return queueFamilyPresentIndex; }
    static inline unsigned int getScrWidth(){return SCR_WIDTH;}
    static inline unsigned int getScrHeight(){return SCR_HEIGHT;}
    static inline unsigned int getCurrentFrame() { return currentFrame; }

    static inline Smoothie::Drawing_Base* getDrawerClass() { return drawerClass.get(); }

    //Generates a new random value between 1 to 2,147,483,648
    //used for creating IDs of various objects
    static unsigned int generate_random_key();

    static inline void setViewport(VkCommandBuffer commandBuffer) 
    {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(SCR_WIDTH);
        viewport.height = static_cast<float>(SCR_HEIGHT);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    };
    static inline void setScissor(VkCommandBuffer commandBuffer) 
    {
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent.width = SCR_WIDTH;
        scissor.extent.height = SCR_HEIGHT;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    static inline Smoothie::DefaultTexture2D getDefault2DTexture() { return default2dTexture; }
    static inline Smoothie::DefaultBuffer getDefaultBuffer() { return defaultBuffer; }
    
    
    SmoothieCore(const SmoothieCore&) = delete;
private:

    static std::shared_ptr<Smoothie::SmoothieCore_Initialization> init_info;
    static std::shared_ptr<Smoothie::Drawing_Base> drawerClass;

    SmoothieCore() = default;
    static std::array<VkCommandBuffer, SMOOTHIE_MAX_FRAMES_IN_FLIGHT> renderCommandBuffers;
    static std::array<VkSemaphore, SMOOTHIE_MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
    static std::array<VkSemaphore, SMOOTHIE_MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
    static std::array<VkFence, SMOOTHIE_MAX_FRAMES_IN_FLIGHT> inFlightFences;
    static unsigned int currentFrame;


    static VkInstance instance;
    static VkPhysicalDevice physicalDevice;
    static VkDevice device;
    static VkSurfaceKHR surface;
    static VkCommandPool commandPool;

    static Smoothie::DefaultTexture2D default2dTexture;
    static Smoothie::DefaultBuffer defaultBuffer;

    static unsigned int queueFamilyGraphicsIndex;
    static VkQueue graphicsQueue;

    static unsigned int queueFamilyPresentIndex;
    static VkQueue presentQueue;

    static VkSwapchainKHR swapchain;
    static std::vector<VkImage> swapchainImages;
    static std::vector<VkImageView> swapchainImageViews;
    static std::vector<VkFramebuffer> swapchainFramebuffers;
    static VkRenderPass defaultRenderPass;

    static bool isEngineReady;
    static std::shared_ptr<Smoothie::Scene_Default> scene;

    static unsigned int SCR_WIDTH, SCR_HEIGHT;
    static SmoothieMath::Matrix4x4 cameraProjectionViewMatrix;

    static std::vector<Smoothie::CameraDescriptorSet> cameraDescriptorSets;
    
    static VmaAllocator vmaAllocator;

    static int create_renderer();
    static int destroy_renderer();

};