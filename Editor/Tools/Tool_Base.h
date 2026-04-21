#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include "Smoothie.h"

namespace SmoothieEditor
{

    enum class BaseTool_Flags: int
    {
        isWindowVisible = 1 << 0,
        isToolUpdated = 1 << 1
    };
    using ToolFlags = int;

    class Core;
    class Tool_Base
    {
    protected:
        std::shared_ptr<SmoothieEditor::Core> m_pCore;
        ToolFlags m_toolFlags = 0;

        inline void update_tool() {m_toolFlags |= static_cast<ToolFlags>(BaseTool_Flags::isToolUpdated);}

    public:

        virtual int create() = 0;
        virtual int on_resize() {return 0;}
        virtual void on_ui_window_draw() {}
        virtual void on_ui_icon_draw() {}
        virtual void on_command_recording_compute(VkCommandBuffer commandBuffer) {}
        virtual void on_command_recording_graphics(VkCommandBuffer commandBuffer) {}
        virtual void on_command_execution_finish() {}
        virtual void destroy() = 0;

        virtual void on_scene_load(const Smoothie::XML::Element& rootElement) {};
        virtual void on_scene_save(Smoothie::XML::Element& rootElement) {};

        //Color picking function.
        virtual void on_screen_pick_record(VkCommandBuffer commandBuffer) {}

        inline bool isToolUpdated() const {return m_toolFlags & static_cast<ToolFlags>(BaseTool_Flags::isToolUpdated);}

        virtual ~Tool_Base() = default;
    };


}