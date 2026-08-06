#pragma once
#include "Common.h"

namespace Smoothie::Editor
{
    struct Tools_Data
    {
        Camera camera;

        bool isMainWindowCovered = false;
        bool isCameraLocked = false;


        virtual ~Tools_Data() = default;
    };


    class Tool_Base
    {
    protected:
        const std::shared_ptr<Tools_Data> g_GlobalData;
    public:
        explicit Tool_Base(const std::shared_ptr<Tools_Data>& globalData) : g_GlobalData(globalData) {};

        virtual int create() = 0;
        virtual int on_resize() {return 0;}
        virtual void on_ui_window_draw() {}
        virtual void on_ui_icon_draw() {}
        virtual void on_command_recording_compute(VkCommandBuffer commandBuffer) {}
        virtual void on_command_recording_graphics(VkCommandBuffer commandBuffer) {}
        virtual void on_command_execution_finish() {}
        virtual void destroy() = 0;

        virtual void on_scene_load(const std::string& file) {};
        virtual void on_scene_save(const std::string& file) {};

        //Color picking function.
        virtual void on_screen_pick_record(VkCommandBuffer commandBuffer) {}

        virtual ~Tool_Base() = default;
    };


}