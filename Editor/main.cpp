#include <iostream>
#include "Demo.h"
#include "Smoothie.h"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"
#include "EditorCore/RenderingManager.h"
#include "EditorCore/Core.h"

const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 720;

GLFWwindow* window = nullptr;

static void resizeCallback(GLFWwindow* window, int width, int height)
{
	SmoothieCore::updateRenderingResolution(width, height);
}


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

int main()
{
	if (!glfwInit())
	{
		std::cout << "Can't initilize glfw!" << std::endl;
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


	//Init engine
	std::shared_ptr<Smoothie::SmoothieCore_Initialization> initInfoBase = std::make_shared<SmoothieEngineInitInfo>();
	SmoothieEngineInitInfo* data = dynamic_cast<SmoothieEngineInitInfo*>(initInfoBase.get());
	data->window = window;
	auto& __core = SmoothieEditor::getCore();
	if (SmoothieCore::initEngine(initInfoBase, WINDOW_WIDTH, WINDOW_HEIGHT, 
		std::make_shared<Smoothie::Scene_Default>(), 
		__core.draw_manager) != 0)
	{
		std::cout << "Failed to initialize the engine!" << std::endl;
		return -1;
	}
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // optional
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // optional
	
	//Disabled for now because of the different swapchain color formats with new viewports.
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(window, true);
	
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.ApiVersion = VK_API_VERSION_1_3;
	init_info.Instance = SmoothieCore::getInstance();
	init_info.PhysicalDevice = SmoothieCore::getPhysicalDevice();
	init_info.Device = SmoothieCore::getDevice();
	init_info.QueueFamily = SmoothieCore::getQueueFamilyGraphicsIndex();
	init_info.Queue = SmoothieCore::getGraphicsQueue();
	init_info.PipelineCache = nullptr;
	init_info.DescriptorPool = nullptr;
	init_info.MinImageCount = SmoothieCore::getSwapchainImages().size();
	init_info.ImageCount = SmoothieCore::getSwapchainImages().size();
	init_info.RenderPass = SmoothieCore::getDefaultRenderPass();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr; 
	init_info.DescriptorPoolSize = 8;
	ImGui_ImplVulkan_Init(&init_info);
	if (__core.icons.create() != 0)
	{ 
		std::cout << "Failed to load icons!" << std::endl;
		return 1;
	}
	__core.draw_manager->create_imgui_data();
	SmoothieCore::loadScene("resources/DemoScene/Demo.sscene");

	io.Fonts->AddFontFromFileTTF("Editor\\fonts\\Roboto-Medium.ttf", 16.0f);
	setup_colors();
	SmoothieEditor::getCore().editor_window.camera.setCameraPosition({ -0.5, 3.0f, 18.0f });
	SmoothieEditor::getCore().editor_window.camera.setCameraFront({ 0.0f, 0.0f, -1.0f });
	SmoothieEditor::getCore().editor_window.camera.setCameraUp({ 0.0f, 1.0f, 0.0f });
	//SmoothieEditor::getCore().editor_window.camera.updateProjectionMatrix(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
	SmoothieEditor::getCore().editor_window.camera.updateCameraMatrices();
	while (!glfwWindowShouldClose(window))
	{
		SmoothieCore::draw();
		SmoothieEditor::getCore().editor_window.on_submit_time();
		glfwPollEvents();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	vkDeviceWaitIdle(SmoothieCore::getDevice());
	__core.draw_manager->destroy_imgui_data();
	__core.icons.destroy();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	SmoothieCore::removeScene();
	SmoothieCore::finitEngine();
	glfwTerminate();
}