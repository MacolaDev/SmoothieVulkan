#pragma once
#include "../Smoothie.h"
#include "GLFW/glfw3.h"
namespace Smoothie::App_Utilities
{
    int create_instance(EngineInitInfo& init_info, bool useValidationLayers);
    void destroy_instance(EngineInitInfo& init_info);

    int create_surface(EngineInitInfo& init_info, GLFWwindow* window);
    void destroy_surface(EngineInitInfo& init_info);

    int select_physical_device(EngineInitInfo& init_info, unsigned int device_index);

    int create_device_and_queues(EngineInitInfo& init_info);
    void destroy_device_and_queues(EngineInitInfo& init_info);

    int create_swapchain(EngineInitInfo& init_info, unsigned int width, unsigned int height);
    void destroy_swapchain(EngineInitInfo& init_info);
}

