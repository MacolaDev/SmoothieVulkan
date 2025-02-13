#include "Demo.h"

using namespace SmoothieMath;
constexpr float apsect_ratio = static_cast<float>(1280.0f / 720.0f);
Smoothie::Camera FreeCamera::freeCamera = Smoothie::Camera(
    { -0.5, 3.0f, 18.0f },
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
