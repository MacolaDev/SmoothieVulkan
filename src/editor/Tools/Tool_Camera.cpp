//
// Created by macola on 12/13/25.
//

#include "Tool_Camera.h"

using namespace Smoothie::Editor;

int Tool_Camera::create()
{
    //m_pCore = std::dynamic_pointer_cast<SmoothieEditor::Core>(SmoothieCore::getDrawingClassPtr()->get_editor_corePtr());
    //assert(m_pCore != nullptr);

    const auto& __drawing_class = std::dynamic_pointer_cast<Smoothie::DeferredRendering::Drawing>(SmoothieCore::getDrawingClassPtr());
    assert(__drawing_class != nullptr);

    //const auto __sampler = __drawing_class->getSampler("ClampToEdgeLINEAR");
    // assert(__sampler != nullptr);
    // m_Icon.sampler = __sampler;
    //return m_Icon.create();
    return 0;
}

void Tool_Camera::destroy()
{
    // m_pCore.reset();
    //m_Icon.destroy();
}

void Tool_Camera::on_ui_window_draw()
{

    //***************************** Draw window ****************************//
    ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Camera navigation");
    ImGui::DragFloat("Camera Speed", &m_speed, 0.1f, 1.0f, 10.0f);
    ImGui::DragFloat("Camera Sensitivity", &m_sensitivity, 0.1f, 0.1f, 1.0f);
    if (ImGui::DragFloat("Camera FOV", &m_fov, 0.1f, 1.0f, 120.0f))
    {
        m_camera.setFov(m_fov);
    }
    ImGui::End();
}

void Tool_Camera::on_ui_icon_draw()
{
	m_mousePosLast = m_mousePos;
	m_mousePos = ImGui::GetMousePos();

	m_winPosLast = m_winPos;
	m_winPos = ImGui::GetWindowPos();

	m_winSizeLast = m_winSize;
	m_winSize = ImGui::GetWindowSize();
	bool __isMouseInsideWindow = (m_mousePos.x >= m_winPos.x && m_mousePos.x <= m_winPos.x + m_winSize.x &&
		m_mousePos.y >= m_winPos.y && m_mousePos.y <= m_winPos.y + m_winSize.y);

	auto& io = ImGui::GetIO();
	m_MouseWheelLast = m_MouseWheelPos;
	m_MouseWheelPos= io.MouseWheel;

	const double __time = ImGui::GetTime();
	const double deltaTime = __time - m_lastTime;
	m_lastTime = __time;


	m_mousePosWin.x = m_mousePos.x - m_winPos.x;
	m_mousePosWin.y = m_mousePos.y - m_winPos.y;
	float __mousePosWinDelta_x = m_mousePosWinLast.x - m_mousePosWin.x;
	float __mousePosWinDelta_y = m_mousePosWinLast.y - m_mousePosWin.y;
	m_mousePosWinLast = m_mousePosWin;

	//Camera moving
	const bool __middleMouseActive = io.MouseDown[2];
	if (__isMouseInsideWindow && (__middleMouseActive))
	{
		float speed = m_speed * deltaTime;
		auto _pos_vec = m_camera.getCameraPosition();

		const auto& _front_vec = m_camera.getCameraFront();
		const auto& _up_vec = m_camera.getCameraUp();
		auto _right_vec = glm::cross(_front_vec, _up_vec);

		if (ImGui::IsKeyDown(ImGuiKey_W))
			_pos_vec += _front_vec * speed;

		if (ImGui::IsKeyDown(ImGuiKey_S))
			_pos_vec -= _front_vec * speed;

		if (ImGui::IsKeyDown(ImGuiKey_A))
			_pos_vec -= glm::normalize(_right_vec) * speed;

		if (ImGui::IsKeyDown(ImGuiKey_D))
			_pos_vec += glm::normalize(_right_vec) * speed;

		m_camera.setCameraPosition(_pos_vec);
	}

	//rotations
	if (__isMouseInsideWindow && (__middleMouseActive))
	{
		float __xoffset = __mousePosWinDelta_x;
		float __yoffset = -__mousePosWinDelta_y;
		__xoffset *= m_sensitivity;
		__yoffset *= m_sensitivity;

		m_yaw += __xoffset;
		m_pitch += __yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (m_pitch > 89.0f)
			m_pitch = 89.0f;
		if (m_pitch < -89.0f)
			m_pitch = -89.0f;

		const float __rad_yaw = m_yaw * 3.141592f / 180.0f;
		const float __rad_pitch = m_pitch * 3.141592f / 180.0f;
		glm::vec3 front;
		front.x = cos(__rad_yaw) * cos(__rad_pitch);
		front.y = sin(__rad_pitch);
		front.z = sin(__rad_yaw) * cos(__rad_pitch);
		front = glm::normalize(front);
		m_camera.setCameraFront(front);
	}

	//resize window update
	if ((m_winSize.x != m_winSizeLast.x) || (m_winSize.y != m_winSizeLast.y))
	{
		m_camera.setAspectRatio(m_winSize.x / m_winSize.y);
	}

    m_camera.updateCameraMatrices();
    SmoothieCore::updateCameraData(m_camera);


    ImGui::SetCursorPos({ 10, 10 });
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);
    //if (ImGui::ImageButton("CameraButton", m_Icon.TextureID, {35, 35}))
    if (ImGui::Button("CameraButton", {35, 35}))
    {
    	//m_toolFlags ^= static_cast<int>(BaseTool_Flags::isWindowVisible);
    }
    ImGui::PopStyleVar(2);
}

void Tool_Camera::on_command_execution_finish()
{
	//SmoothieCore::updateCameraData(m_camera);
}

void Tool_Camera::on_scene_load(const std::string& file)
{
	// if (rootElement.hasChild("Tool_Camera"))
	// {
	// 	const auto& __Tool_Camera_Element = rootElement.getChild("Tool_Camera");
	// 	if (__Tool_Camera_Element.hasChild("m_fov"))
	// 	{
	// 		m_fov = __Tool_Camera_Element.getChild("m_fov").getFloat();
	// 	}
	//
	// 	if (__Tool_Camera_Element.hasChild("m_sensitivity"))
	// 	{
	// 		m_sensitivity = __Tool_Camera_Element.getChild("m_sensitivity").getFloat();
	// 	}
	//
	// 	if (__Tool_Camera_Element.hasChild("m_speed"))
	// 	{
	// 		m_speed = __Tool_Camera_Element.getChild("m_speed").getFloat();
	// 	}
	//
	// 	bool __updateCameraData = false;
	// 	if (__Tool_Camera_Element.hasChild("m_camera.position"))
	// 	{
	// 		SmoothieMath::Vector3 __vec =  __Tool_Camera_Element.getChild("m_camera.position").getVector3();
	// 		m_camera.setCameraPosition(__vec);
	// 		__updateCameraData = true;
	// 	}
	//
	// 	if (__Tool_Camera_Element.hasChild("m_camera.front"))
	// 	{
	// 		SmoothieMath::Vector3 __vec =  __Tool_Camera_Element.getChild("m_camera.front").getVector3();
	// 		m_camera.setCameraFront(__vec);
	// 		__updateCameraData = true;
	// 	}
	//
	// 	if (__Tool_Camera_Element.hasChild("m_camera.up"))
	// 	{
	// 		SmoothieMath::Vector3 __vec =  __Tool_Camera_Element.getChild("m_camera.up").getVector3();
	// 		m_camera.setCameraUp(__vec);
	// 		__updateCameraData = true;
	// 	}
	//
	// 	if (__updateCameraData) {m_camera.updateCameraMatrices();}
	//
	// }
}

static std::string _Vector3_to_string(const glm::vec3& vec)
{
	return std::to_string(vec.x) + " " + std::to_string(vec.y) + " " + std::to_string(vec.z);
}

void Tool_Camera::on_scene_save(const std::string &file)
{
	// using Element = Smoothie::XML::Element;
	// Element _Tool_Camera_Element("Tool_Camera");
	//
	// _Tool_Camera_Element.children.resize(6);
	//
	// _Tool_Camera_Element.children[0] = Element("m_fov", std::to_string(m_fov));
	// _Tool_Camera_Element.children[1] = Element("m_sensitivity", std::to_string(m_sensitivity));
	// _Tool_Camera_Element.children[2] = Element("m_speed", std::to_string(m_speed));
	//
	// _Tool_Camera_Element.children[3] = Element("m_camera.position", _Vector3_to_string(m_camera.getCameraPosition()));
	// _Tool_Camera_Element.children[4] = Element("m_camera.front", _Vector3_to_string(m_camera.getCameraFront()));
	// _Tool_Camera_Element.children[5] = Element("m_camera.up", _Vector3_to_string(m_camera.getCameraUp()));
	//
	// rootElement.children.push_back(_Tool_Camera_Element);
}
