#include "EditorWidgets.h"
#include "Core.h"

#include "backends/imgui_impl_vulkan.h"

#include <iostream>
#include <array>
#include <algorithm>

void SmoothieEditor::Window_Editor::on_draw_time(VkCommandBuffer commandBuffer, unsigned int current_frame)
{
	isWindowSelected = ImGui::IsWindowFocused();

	mousePosLast = mousePos;
	mousePos = ImGui::GetMousePos();

	winPosLast = winPos;
	winPos = ImGui::GetWindowPos();

	winSizeLast = winSize;
	winSize = ImGui::GetWindowSize();
	isMouseInsideWindow = (mousePos.x >= winPos.x && mousePos.x <= winPos.x + winSize.x &&
		mousePos.y >= winPos.y && mousePos.y <= winPos.y + winSize.y);

	auto& io = ImGui::GetIO();
	lastMouseWheelPos = MouseWheelPos;
	MouseWheelPos = io.MouseWheel;
	const double time = ImGui::GetTime();
	deltaTime = time - lastTime;
	lastTime = time;


	mousePosWin.x = mousePos.x - winPos.x;
	mousePosWin.y = mousePos.y - winPos.y;
	mousePosWinDelta.x = mousePosWinLast.x - mousePosWin.x;
	mousePosWinDelta.y = mousePosWinLast.y - mousePosWin.y;
	mousePosWinLast = mousePosWin;

	//Camera moving
	const bool _middleMouseActive = io.MouseDown[2];
	if (isMouseInsideWindow && (_middleMouseActive))
	{
		float speed = camera_window.speed * deltaTime;
		auto _pos_vec = camera.getCameraPosition();

		const auto& _front_vec = camera.getCameraFront();
		const auto& _up_vec = camera.getCameraUp();
		auto _right_vec = SmoothieMath::cross(_front_vec, _up_vec);

		if (ImGui::IsKeyDown(ImGuiKey_W))
			_pos_vec += _front_vec * speed;

		if (ImGui::IsKeyDown(ImGuiKey_S))
			_pos_vec -= _front_vec * speed;

		if (ImGui::IsKeyDown(ImGuiKey_A))
			_pos_vec -= SmoothieMath::normalize(_right_vec) * speed;

		if (ImGui::IsKeyDown(ImGuiKey_D))
			_pos_vec += SmoothieMath::normalize(_right_vec) * speed;

		camera.setCameraPosition(_pos_vec);
	}

	//rotations
	if (isMouseInsideWindow && (_middleMouseActive))
	{
		float xoffset = mousePosWinDelta.x;
		float yoffset = -mousePosWinDelta.y;
		xoffset *= camera_window.sensitivity;
		yoffset *= camera_window.sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		float rad_yaw = yaw * 3.141592f / 180.0f;
		float rad_pitch = pitch * 3.141592f / 180.0f;
		SmoothieMath::Vector3 front;
		front.x = cos(rad_yaw) * cos(rad_pitch);
		front.y = sin(rad_pitch);
		front.z = sin(rad_yaw) * cos(rad_pitch);
		front.normalizeVector();
		camera.setCameraFront(front);
	}

	//resize window update
	if ((winSize.x != winSizeLast.x) || (winSize.y != winSizeLast.y))
	{
		camera.setAspectRatio(winSize.x / winSize.y);
	}
	camera.updateCameraMatrices();
	
	
	ImGui::SetCursorPos({ 10, 10 });
	
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);
	if (ImGui::ImageButton("CameraButton", getCore().icons.camera_icon.TextureID, {35, 35}))
	{
		isCameraWindowVisible = !isCameraWindowVisible;
	}

	ImGui::SetCursorPos({ 10, 35 + 10 + 5 + 5 + 5});
	if (ImGui::ImageButton("ObjectSelectButton", getCore().icons.object_select_icon.TextureID, { 35, 35 }))
	{
		
	}

	ImGui::PopStyleVar(2);
	
	NDCWindow.x = std::clamp((mousePosWin.x / winSize.x), 0.0f, 1.0f);
	NDCWindow.y = 1.0f - std::clamp((mousePosWin.y / winSize.y), 0.0f, 1.0f);


	ImGui::ShowDemoWindow();

}

void SmoothieEditor::Window_Editor::on_submit_time()
{
	SmoothieCore::updateCameraData(camera);
	camera_window.on_submit_time();
}

void SmoothieEditor::Camera_Window::draw_ui()
{
	ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_NoCollapse);
	ImGui::Text("Camera navigation");
	ImGui::DragFloat("Camera Speed", &speed, 0.1f, 1.0f, 10.0f);
	ImGui::DragFloat("Camera Sensitivity", &sensitivity, 0.1f, 0.1f, 1.0f);
	if (ImGui::DragFloat("Camera FOV", &fov, 0.1f, 1.0f, 120.0f))
	{
		getCore().editor_window.camera.setFov(fov);
	}

	ImGui::Separator();
	ImGui::Text("View options");
	const char* _preview_val = __renderpass_names[__selected_pass];
	if (ImGui::BeginCombo("Draw Pass", _preview_val))
	{
		for (size_t i = 0; i < IM_ARRAYSIZE(__renderpass_names); i++)
		{
			const bool __is_selected = (__selected_pass == i);
			if (ImGui::Selectable(__renderpass_names[i], &__is_selected))
			{
				__selected_pass = i;
			}

			if (__is_selected) ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}
	

	ImGui::End();
}

void SmoothieEditor::Camera_Window::on_submit_time()
{
	
	if (__selected_pass != __last_selected_pass)
	{
		vkQueueWaitIdle(SmoothieCore::getGraphicsQueue());
		getCore().draw_manager->update_swizzler(__selected_pass);
		__last_selected_pass = __selected_pass;
	}
}

