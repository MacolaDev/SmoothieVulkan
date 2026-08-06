// #pragma once
// #include "Common.h"
// #include "Utilities/Icons.h"
//
// #include "Tools/Tool_Base.h"
// #include "Tools/Tool_Camera.h"
// #include "Tools/Tool_ScreenPicker.h"
// #include "Tools/Tool_SceneGraph.h"
//
// namespace SmoothieEditor
// {
//
//     class Core
//     {
//     protected:
//
//         std::shared_ptr<Smoothie::DeferredRendering::Drawing> m_drawing_class;
//         VkImage m_resultImage = nullptr;
//         VkImageView m_resultImageView = nullptr;
//         VmaAllocation m_resultImageAllocation = nullptr;
//         VkDescriptorSet m_resultImageTextureID = nullptr;
//         VkPipeline m_resultPipeline = nullptr;
//
//         struct ToolsSearchKey
//         {
//             std::string name;
//             int priority;
//
//             inline bool operator<(const ToolsSearchKey& key) const{return priority <= key.priority;}
//         };
//
//         const std::vector<const char*> m_editor_shader_files =
//         {
//             "shaders/Editor/swizzler.sshader",
//             "shaders/Editor/utilities.sshader"
//         };
//         std::unordered_map<std::string, VkShaderModule> m_editor_shaders;
//     public:
//
//         int resize_callback();
//         void on_scene_load(const std::string &settings_file);
//         void on_scene_save(const std::string &settings_file);
//         // inline decltype(m_tools)& get_tools() { return m_tools; }
//         inline VkShaderModule get_editor_shader_module(const std::string& name) { return m_editor_shaders.find(name) != m_editor_shaders.end() ? m_editor_shaders[name] : nullptr; }
//         int create();
//         void on_command_record_time(VkCommandBuffer commandBuffer, unsigned imageIndex);
//         void on_command_submit_time();
//         void destroy();
//     };
// }
