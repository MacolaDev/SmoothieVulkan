#include <iostream>

#include "Demo.h"

#include "Smoothie.h"
#include "imgui.h"

#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

#include "GLFW/glfw3.h"

#include "EditorCore.h"

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
		std::cout << "Can't initialize glfw!" << std::endl;
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


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForVulkan(window, true);
	io.Fonts->AddFontFromFileTTF("Editor/fonts/Roboto-Medium.ttf", 16.0f);
	setup_colors();

	//Init engine
	std::shared_ptr<Smoothie::SmoothieCore_Initialization> initInfoBase = std::make_shared<SmoothieEngineInitInfo>();
	auto* data = dynamic_cast<SmoothieEngineInitInfo*>(initInfoBase.get());
	data->window = window;
	std::shared_ptr<Smoothie::Drawing_Base> renderingManager = std::make_shared<Smoothie::DeferredRendering::Drawing>();
	renderingManager->set_editor_core(std::make_shared<SmoothieEditor::Core>());

	if (SmoothieCore::initEngine(initInfoBase, WINDOW_WIDTH, WINDOW_HEIGHT, 
		std::make_shared<Smoothie::DeferredRendering::Deferred_Scene>(),
		renderingManager) != 0)
	{
		std::cout << "Failed to initialize the engine!" << std::endl;
		return -1;
	}


	SmoothieCore::loadScene("resources/DemoScene/Demo.sscene");
	while (!glfwWindowShouldClose(window))
	{
		SmoothieCore::draw();
		glfwPollEvents();
	}
	vkDeviceWaitIdle(SmoothieCore::getDevice());
	SmoothieCore::removeScene();

	SmoothieCore::finitEngine();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
}