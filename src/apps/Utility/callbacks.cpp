//
// Created by macola on 7/20/26.
//
#include "callbacks.h"

using namespace Smoothie;

Camera App_Utilities::defaultCameraSetup()
{
    Camera camera;
    camera.setCameraPosition({-0.5, 3.0f, 18.0f});
    camera.setCameraFront({ 0.0f, 0.0f, -1.0f });
    camera.setCameraUp({ 0.0f, 1.0f, 0.0f });
    camera.setFov(glm::radians(45.0f));
    camera.setAspectRatio(1280.0f / 720.0f);
    camera.setZNear(0.1f);
    camera.setZFar(100.0f);
    camera.updateCameraMatrices();
    return camera;
}

static float yaw = -90.0f;
static float pitch = 0.0f;
static float lastX = 800.0f / 2.0;
static float lastY = 600.0f / 2.0;
static float fov = 45.0f;
static bool firstMouse = true;
static float cameraSpeed = 3;
static float deltaTime = 0.0f;
static float currentFrame = 0.0f;
static float lastFrame = 0.0f;
static Camera freeCamera = App_Utilities::defaultCameraSetup();

void App_Utilities::Callback_Mouse(GLFWwindow* window, double xposIn, double yposIn)
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

    glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
    direction.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    direction.y = glm::sin(glm::radians(pitch));
    direction.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    direction = glm::normalize(direction);

    freeCamera.setCameraFront(direction);
    freeCamera.updateCameraMatrices();
    SmoothieCore::updateCameraData(freeCamera);
}

void App_Utilities::Callback_Resize(GLFWwindow *window_, int width, int height)
{
    const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    freeCamera.setAspectRatio(aspectRatio);
    SmoothieCore::updateCameraData(freeCamera);
}

void App_Utilities::Callback_Keyboard(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    //Press ESC to close Demo
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    auto cameraPosition = freeCamera.getCameraPosition();

    float speed = cameraSpeed * deltaTime;

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
        cameraPosition -= glm::normalize(glm::cross(freeCamera.getCameraFront(), freeCamera.getCameraUp())) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraPosition += glm::normalize(glm::cross(freeCamera.getCameraFront(), freeCamera.getCameraUp())) * speed;
    }

    freeCamera.setCameraPosition(cameraPosition);
    freeCamera.updateCameraMatrices();
    SmoothieCore::updateCameraData(freeCamera);
}

void App_Utilities::Callback_GLFWError(int error_code, const char *description)
{
    SmoothieCore::logCritical_Formated("[GLFW]: Error: {}; {}", error_code, description);
}

void App_Utilities::update_time()
{
    currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

