//
// Created by macola on 12/25/25.
//

#include "Tool_SceneGraph.h"
#include "imgui.h"

int SmoothieEditor::Tool_SceneGraph::create()
{
    return 0;
}

void SmoothieEditor::Tool_SceneGraph::destroy()
{

}

void SmoothieEditor::Tool_SceneGraph::on_ui_window_draw()
{
    ImGui::Begin("Scene Graph");
    if (ImGui::TreeNode("Models"))
    {

        ImGui::TreePop();
    }
    ImGui::End();
}
