#pragma once
#include "../Smoothie.h"
#include "GLFW/glfw3.h"

namespace Smoothie::App_Utilities
{
    void Callback_Mouse(GLFWwindow* window, double xposIn, double yposIn);
    void Callback_Resize(GLFWwindow* window_, int width, int height);
    void Callback_Keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
    void Callback_GLFWError(int error_code, const char* description);
    void update_time();
    Camera defaultCameraSetup();
}

