#pragma once

#include "Tool_Base.h"
#include "../Utilities/Icons.h"

namespace Smoothie::Editor
{
    class Tool_Camera: public Tool_Base
    {
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

        // Icon m_Icon = Icon("Editor/icons/camera_icon.png", nullptr);

    public:
        explicit Tool_Camera(const std::shared_ptr<Tools_Data>& globalData) : Tool_Base(globalData) {};

        inline float getSpeed() const {return m_speed;}
        inline float getSensitivity() const {return m_sensitivity;}
        inline float getFov() const {return m_fov;}
        inline const Smoothie::Camera& getCamera() const {return m_camera;}

        int create() override;
        void on_ui_window_draw() override;
        void on_ui_icon_draw() override;
        void on_command_execution_finish() override;
        void on_scene_load(const std::string &file) override;
        void on_scene_save(const std::string &rootElement) override;

        void destroy() override;
    };
}