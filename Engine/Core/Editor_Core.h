#pragma once
#include "Common.h"

namespace Smoothie
{
    //Base class for engine editor.
    class Editor_Core
    {
    public:
        virtual int create() = 0;
        virtual void on_command_record_time(VkCommandBuffer commandBuffer, unsigned int imageIndex) = 0;
        virtual void on_command_submit_time() = 0;
        virtual void destroy() = 0;

        virtual void on_scene_load(const std::string& settings_file) = 0;
        virtual void on_scene_save(const std::string& settings_file) = 0;

        virtual int resize_callback() = 0;

        virtual ~Editor_Core() = default;
    };

}