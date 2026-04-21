#pragma once

#include "Tool_Base.h"
#include "../Utilities/Icons.h"
#include "imgui.h"

namespace SmoothieEditor
{
    class Tool_Camera: public Tool_Base
    {

    private:
        float m_fov = 45.0f;
        float m_sensitivity = 0.1f;
        float m_speed = 3.0f;
        Smoothie::Camera m_camera;


        float m_MouseWheelPos = 0.0f;
        float m_MouseWheelLast = 0.0f;
        ImVec2 m_mousePos;
        ImVec2 m_mousePosLast;
        ImVec2 m_winPos;
        ImVec2 m_winPosLast;
        ImVec2 m_winSize;
        ImVec2 m_winSizeLast;
        ImVec2 m_mousePosWin;
        ImVec2 m_mousePosWinLast;


        float m_yaw = -90.0f;
        float m_pitch = 0.0f;
        double m_lastTime = 0;

        Icon m_Icon = Icon("Editor/icons/camera_icon.png", nullptr);

    public:

        inline float getSpeed() const {return m_speed;}
        inline float getSensitivity() const {return m_sensitivity;}
        inline float getFov() const {return m_fov;}
        inline const Smoothie::Camera& getCamera() const {return m_camera;}

        int create() override;
        void on_ui_window_draw() override;
        void on_ui_icon_draw() override;
        void on_command_execution_finish() override;
        void on_scene_load(const Smoothie::XML::Element &rootElement) override;
        void on_scene_save(Smoothie::XML::Element &rootElement) override;

        void destroy() override;
    };
}