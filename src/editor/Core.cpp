#include "Core.h"

#include "imgui_internal.h"

using namespace Smoothie::Editor;

int Core::create()
{
    VkSamplerCreateInfo _image_sampler_create_info{};
    _image_sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    _image_sampler_create_info.magFilter = VK_FILTER_LINEAR;
    _image_sampler_create_info.minFilter = VK_FILTER_NEAREST;
    _image_sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    _image_sampler_create_info.addressModeU = _image_sampler_create_info.addressModeV = _image_sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    if (vkCreateSampler(SmoothieCore::getDevice(), &_image_sampler_create_info, nullptr, &m_Sampler))
    {
        SMOOTHIE_CRITICAL_WITH_SRC("Failed to create sampler.");
        return 1;
    }

    assert(m_Sampler != nullptr);
    assert(m_Result_ImageView != nullptr);
    m_ResultImage_Set = ImGui_ImplVulkan_AddTexture(m_Sampler, m_Result_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


    return 0;
}

int Core::resize_callback()
{
    ImGui_ImplVulkan_RemoveTexture(m_ResultImage_Set);
    assert(m_Sampler != nullptr);
    assert(m_Result_ImageView != nullptr);
    m_ResultImage_Set = ImGui_ImplVulkan_AddTexture(m_Sampler, m_Result_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    return 0;
}

void Core::ui()
{
    ImGui::NewFrame();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = { 0, 0 };

    const auto& _id = ImGui::GetID("DockingSpaceID");
    ImGui::DockSpaceOverViewport(_id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoDockingOverCentralNode | ImGuiDockNodeFlags_AutoHideTabBar, nullptr);

    assert(m_ResultImage_Set != nullptr);

    //This thing disables tab bar from showing in the main note.
    //We dont need it anyway since no window can be docked to it anyway since its a central node.
    //This is from internal API for docking, I hope it wont change in the future :D
    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoTabBar;
    ImGui::SetNextWindowClass(&window_class);
    ImGui::SetNextWindowDockID(_id);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 1);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 2, 2 });
    ImGui::Begin("Canvas1", nullptr, 0);

    ImGui::BeginTabBar("Options1");

    int _button_count = 10;
    ImVec2 _button_size = {40, 40};
    ImVec2 _window_size = {0, 40};
    if (_button_count * _button_size.x > ImGui::GetWindowSize().x)
    {
        _window_size.y += 15;
    }

    if (ImGui::BeginTabItem("General"))
    {
        ImGui::BeginChild("Child4", _window_size,false, ImGuiWindowFlags_HorizontalScrollbar);
        for (int i = 0; i < _button_count; i++)
        {
            ImGui::PushID(i);
            ImGui::Button("Testing2", _button_size);
            ImGui::SameLine();
            ImGui::PopID();
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();


    ImGui::Image(m_ResultImage_Set, ImGui::GetContentRegionAvail());

    ImGui::End();
    ImGui::PopStyleVar(2);

    ImGui::ShowDemoWindow();

    tools_work_ui();

    ImGui::Render();
}

void Core::compute_work(VkCommandBuffer command_buffer)
{
    assert(command_buffer != nullptr);

}

void Core::draw_ui(VkCommandBuffer command_buffer)
{
    assert(command_buffer != nullptr);

    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer);
}

void Core::destroy()
{
    ImGui_ImplVulkan_RemoveTexture(m_ResultImage_Set); m_ResultImage_Set = nullptr;
    if (m_Sampler != nullptr) vkDestroySampler(SmoothieCore::getDevice(), m_Sampler, nullptr);
    m_Sampler = nullptr;
}

void Core::tools_work_ui()
{
    //if (m_Tool_Camera != nullptr) m_Tool_Camera->on_ui_window_draw();
}
