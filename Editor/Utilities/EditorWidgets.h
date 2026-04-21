#pragma once
#include "imgui.h"
#include "Smoothie.h"


namespace SmoothieEditor 
{

	struct Window_Base
	{

	};

	struct Camera_Window
	{
		float fov = 45.0f;
		float sensitivity = 0.1f;
		float speed = 3.0f;

		int __selected_pass = 0;
		int __last_selected_pass = 0;
		void draw_ui();
		void on_submit_time(); //Updates the descriptor sets if they are being ghanged 
	private:
		const char* __renderpass_names[7] =
		{ "Default", "Albedo", "Metalic", "Roughness", "Baked AO", "Normal", "Depth"};
	};


	//Main class behind editor window
	struct Window_Editor
	{
		bool isWindowSelected = false;
		bool isMouseInsideWindow = false;
		bool isLeftMousePressed = false;
		bool isMiddleMousePressed = false;
		bool isRightMousePressed = false;

		bool isCameraWindowVisible = false;

		float MouseWheelPos = 0.0f;
		float lastMouseWheelPos = 0.0f;
		ImVec2 mousePos;
		ImVec2 mousePosLast;
		ImVec2 winPos;
		ImVec2 winPosLast;
		ImVec2 winSize;
		ImVec2 winSizeLast;

		ImVec2 mousePosWin;
		ImVec2 mousePosWinLast;
		ImVec2 mousePosWinDelta;

		ImVec2 NDCWindow;

		float yaw = -90.0f;
		float pitch = 0.0f;
		double lastTime = 0;
		double deltaTime = 0;
		
		Camera_Window camera_window;
		Smoothie::Camera camera;

		void on_draw_time(VkCommandBuffer commandBuffer, unsigned int current_frame);
		void on_submit_time();
	};

}
