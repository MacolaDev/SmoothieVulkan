#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../Smoothie.h"

#include "Utility/init_vulkan.h"
#include "Utility/logger.h"
#include "Utility/callbacks.h"

static constexpr unsigned int WINDOW_WIDTH = 1280;
static constexpr unsigned int WINDOW_HEIGHT = 720;
GLFWwindow* window = nullptr;
Smoothie::EngineInitInfo engine_init_info{};

static int create_engine_data(Smoothie::EngineInitInfo& init_info)
{
    if (Smoothie::App_Utilities::create_instance(init_info, true) != 0) return 1;
    if (Smoothie::App_Utilities::create_surface(init_info, window) != 0) return 1;
    if (Smoothie::App_Utilities::select_physical_device(init_info, 0) != 0) return 1;
    if (Smoothie::App_Utilities::create_device_and_queues(init_info) != 0) return  1;
    if (Smoothie::App_Utilities::create_swapchain(init_info, WINDOW_WIDTH, WINDOW_HEIGHT) != 0) return 1;
    return 0;
}


static void destroy_engine_data(Smoothie::EngineInitInfo& init_info)
{
    Smoothie::App_Utilities::destroy_swapchain(init_info);
    Smoothie::App_Utilities::destroy_device_and_queues(init_info);
    init_info.physical_device = nullptr;
    Smoothie::App_Utilities::destroy_surface(init_info);
    Smoothie::App_Utilities::destroy_instance(init_info);
}

static void resizeCallback(GLFWwindow* window_, int width, int height)
{
    Smoothie::App_Utilities::create_swapchain(engine_init_info, width, height);
    SmoothieCore::updateRenderingResolution(width, height, engine_init_info.swapchain);
}

int main()
{
    Smoothie::App_Utilities::setup_logging("test_logger.txt", true, true);
    SmoothieCore::setLoggingCallback(Smoothie::App_Utilities::logger);
	if (!glfwInit()) 
	{
	    SmoothieCore::logCritical("Can't initialize glfw!");
		return -1;
	}

	glfwSetErrorCallback(Smoothie::App_Utilities::Callback_GLFWError);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "SmoothieVulkan", nullptr, nullptr);
	
	//Callback functions
	glfwSetCursorPosCallback(window, Smoothie::App_Utilities::Callback_Mouse);
	//glfwSetFramebufferSizeCallback(window, Smoothie::App_Utilities::Callback_Resize);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

	//Disable cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (create_engine_data(engine_init_info) != 0)
    {
        destroy_engine_data(engine_init_info);
        std::terminate();
    }

    engine_init_info.width = WINDOW_WIDTH;
    engine_init_info.height = WINDOW_HEIGHT;
    engine_init_info.frames_in_flight = 3;
    engine_init_info.draw_class = std::make_shared<Smoothie::DeferredRendering::Drawing>();
    if (SmoothieCore::initEngine(engine_init_info) != 0)
    {
        //std::cout << "Failed to initialize the engine!" << std::endl;
        return -1;
    }

	SmoothieCore::loadScene("resources/DemoScene/Demo.sscene");
	while (!glfwWindowShouldClose(window))
	{
		SmoothieCore::draw();
	    Smoothie::App_Utilities::Callback_Keyboard(window, 0, 0, 0, 0);
	    Smoothie::App_Utilities::update_time();
		glfwPollEvents();
	}

	SmoothieCore::removeScene();
	SmoothieCore::finitEngine();
    destroy_engine_data(engine_init_info);
	glfwTerminate();
}