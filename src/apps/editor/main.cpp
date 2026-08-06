#define SMOOTHIE_INCLUDE_EDITOR
#include "../Smoothie.h"

#include "backends/imgui_impl_glfw.h"
#include "EditorClass.h"
#include "GLFW/glfw3.h"
#include "Utility/init_vulkan.h"
#include "Utility/logger.h"
#include "Utility/callbacks.h"

static void setup_colors(ImGuiStyle& style = ImGui::GetStyle())
{
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Button] = { 0.3f, 0.3f, 0.3f, 0.4f };
	colors[ImGuiCol_ButtonHovered] = { 0.4f, 0.4f, 0.4f, 0.5f };
	colors[ImGuiCol_ButtonActive] = { 0.35f, 0.35f, 0.35f, 0.45f };

	colors[ImGuiCol_Header] = { 0.2f, .2f, .2f, 0.83f };
	colors[ImGuiCol_HeaderHovered] = { 0.3f, .3f, .3f, 0.83f };
	colors[ImGuiCol_HeaderActive] = { 0.35f, .35f, .35f, 0.83f };

	colors[ImGuiCol_FrameBg] = { 0.2f, .2f, .2f, 0.83f };
	colors[ImGuiCol_FrameBgHovered] = { 0.3f, .3f, .3f, 0.83f };
	colors[ImGuiCol_FrameBgActive] = { 0.35f, .35f, .35f, 0.83f };

	colors[ImGuiCol_TitleBgActive] = { .3f, .0f, .0f, 1.0f };
	colors[ImGuiCol_CheckMark] = { 0.0f, 1.0f, 0.0f, 1.0f };

	colors[ImGuiCol_SliderGrab] = { 0.0f, 0.027f, 1.0f, 1.0f };
	colors[ImGuiCol_SliderGrab] = { 0.0f, 0.027f, 0.8f, 1.0f };

	colors[ImGuiCol_Tab] = { 0.3f, 0.3f, 0.3f, 0.4f };
	colors[ImGuiCol_TabHovered] = { 0.4f, 0.4f, 0.4f, 0.5f };
	colors[ImGuiCol_TabSelected] = { 0.45f, 0.45f, 0.45f, 0.55f };

	colors[ImGuiCol_DockingEmptyBg] = { 0,0,0,0 };
	colors[ImGuiCol_Border] = { 0, 0, 0, 1 };
	style.WindowBorderSize = 0;
	style.ChildBorderSize = 0;
	style.PopupBorderSize = 0;
	style.FrameBorderSize = 0;
	style.TabBorderSize = 0;
	style.TabBarBorderSize = 0;
	style.TabBarOverlineSize = 0;

	style.WindowRounding = 4;
	style.ChildRounding = 4;
	style.FrameRounding = 3;
	style.PopupRounding = 4;
	style.ScrollbarRounding = 12;
	style.GrabRounding = 2;
	style.TabRounding = 4;

	style.WindowPadding = { 0, 0 };
}

static constexpr unsigned int WINDOW_WIDTH = 1280;
static constexpr unsigned int WINDOW_HEIGHT = 720;
static GLFWwindow* window = nullptr;
static Smoothie::EngineInitInfo engine_init_info{};
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

static bool g_FramebufferResized = false;

static void resizeCallback(GLFWwindow* window_, int width, int height)
{
    g_FramebufferResized = true;
    engine_init_info.width = width;
    engine_init_info.height = height;
}

int main()
{
    Smoothie::App_Utilities::setup_logging("test_logger.txt", true, true);
    SmoothieCore::setLoggingCallback(Smoothie::App_Utilities::logger);
    glfwSetErrorCallback(Smoothie::App_Utilities::Callback_GLFWError);
    if (!glfwInit())
	{
		SmoothieCore::logCritical("Can't initialize glfw!");
		return -1;
	}


	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Smoothie Editor", nullptr, nullptr);

	//Callback functions
	//glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetFramebufferSizeCallback(window, resizeCallback);

	//Disable cursor
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (create_engine_data(engine_init_info) != 0)
    {
        SmoothieCore::logCritical("Failed to initialize vulkan data.");
        return -1;
    }

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();
	io.Fonts->AddFontFromFileTTF("Editor/fonts/Roboto-Medium.ttf", 16.0f);
    setup_colors();
    if (!ImGui_ImplGlfw_InitForVulkan(window, true))
    {
        SmoothieCore::logCritical("Failed to initialize ImGui for GLFW.");
        return -1;
    }

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.ApiVersion = SmoothieCore::getRequiredVulkanAPIVersion();
    init_info.Instance = engine_init_info.instance;
    init_info.PhysicalDevice = engine_init_info.physical_device;
    init_info.Device = engine_init_info.device;
    init_info.QueueFamily = engine_init_info.queue_family_index_graphics;
    init_info.Queue = engine_init_info.queue_graphics;
    init_info.DescriptorPoolSize = 32;
    init_info.RenderPass = nullptr;
    init_info.Subpass = 0;
    init_info.MinImageCount = 2;
    init_info.ImageCount = engine_init_info.frames_in_flight;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.UseDynamicRendering = true;

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info = {};
    pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipeline_rendering_create_info.colorAttachmentCount = 1;
    pipeline_rendering_create_info.pColorAttachmentFormats = &engine_init_info.swapchain_image_format;
    init_info.PipelineRenderingCreateInfo = pipeline_rendering_create_info;
    if (!ImGui_ImplVulkan_Init(&init_info))
    {
        SmoothieCore::logCritical("Failed to initialize ImGui for Vulkan.");
        return -1;
    }

    engine_init_info.width = WINDOW_WIDTH;
    engine_init_info.height = WINDOW_HEIGHT;
    engine_init_info.frames_in_flight = 3;
    engine_init_info.draw_class = std::make_shared<EditorClass>();
	if (SmoothieCore::initEngine(engine_init_info) != 0)
	{
	    SmoothieCore::logCritical("Failed to initialize engine.");
		return -1;
	}

	//SmoothieCore::loadScene("resources/DemoScene/Demo.sscene");
    SmoothieCore::updateCameraData(Smoothie::App_Utilities::defaultCameraSetup());
	while (!glfwWindowShouldClose(window))
	{
		SmoothieCore::draw();
		glfwPollEvents();
	    if (g_FramebufferResized)
	    {
	        Smoothie::App_Utilities::create_swapchain(engine_init_info, engine_init_info.width, engine_init_info.height);
	        SmoothieCore::updateRenderingResolution(engine_init_info.width, engine_init_info.height, engine_init_info.swapchain);
	        g_FramebufferResized = false;
	    }
	}
	vkDeviceWaitIdle(SmoothieCore::getDevice());
	SmoothieCore::removeScene();

	SmoothieCore::finitEngine();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
    destroy_engine_data(engine_init_info);
	glfwTerminate();
    return 0;
}