#define GLFW_INCLUDE_VULKAN
#include <iostream>
#include <GLFW/glfw3.h>
#include <Smoothie.h>
#include <vector>
#include "Demo.h"
#include <memory>

#include "../include/glfw/include/GLFW/glfw3.h"

const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 720;

GLFWwindow* window = nullptr;

static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) 
{
	FreeCamera::MouseCallback(window, xposIn, yposIn);
}

static void resizeCallback(GLFWwindow* window, int width, int height) 
{
	FreeCamera::resolutionUpdate(width, height);
	SmoothieCore::updateRenderingResolution(width, height);
}

static void glfwCallbackError(int error_code, const char* description)
{
	std::cout << "GLFW Error: " << error_code << ": " << description << std::endl;
}

int main()
{

	if (!glfwInit()) 
	{
		std::cout << "Can't initilize glfw!" << std::endl;
		return -1;
	}

	glfwSetErrorCallback(glfwCallbackError);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "SmoothieVulkan", nullptr, nullptr);
	
	//Callback functions
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetFramebufferSizeCallback(window, resizeCallback);

	//Disable cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	//Init engine
	std::shared_ptr<Smoothie::SmoothieCore_Initialization> initInfoBase = std::make_shared<SmoothieEngineInitInfo>();
	SmoothieEngineInitInfo* data = dynamic_cast<SmoothieEngineInitInfo*>(initInfoBase.get());
	data->window = window;

	if (SmoothieCore::initEngine(initInfoBase, WINDOW_WIDTH, WINDOW_HEIGHT) != 0)
	{
		std::cout << "Failed to initialize the engine!" << std::endl;
		return -1;
	}

	SmoothieCore::loadScene("resources/DemoScene/Demo.sscene");
	while (!glfwWindowShouldClose(window))
	{
		SmoothieCore::draw();
		FreeCamera::updateTime();
		FreeCamera::KeyboardCallback(window);
		glfwPollEvents();
	}

	SmoothieCore::removeScene();
	SmoothieCore::finitEngine();
	glfwTerminate();
}