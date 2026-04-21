#pragma once

#include "Tool_Base.h"

namespace SmoothieEditor
{
    //Highest level
    class Tool_SceneGraph: public Tool_Base
    {
    public:
        int create() override;

        void destroy() override;

        void on_ui_window_draw() override;
    };
}