// #pragma once
// #include "Tool_Base.h"
//
// namespace SmoothieEditor
// {
//     enum class Tool_ScreenPickerOptionFlag
//     {
//         None = (1 << 0),
//         CursorWorldCoordinate = (1 << 1),
//         ModelIDs = (1 << 2),
//         ToolsIDs = (1 << 3),
//     };
//
//
//     struct ScreenPickData
//     {
//
//         //I decided to use first byte in ModelID as indicator for tool, other 3 bytes can be used for screen picking the models (about 16.7m possible models).
//         unsigned int ModelID = 0;
//
//         //This does not need to hold literal instance id, rather its ment to be used as "subdata" for ModelID.
//         //If for example, tool is that is being drawn is gizmo, then this can hold the different directions of that gizmo, instead of using multiple ModelIDs.
//         unsigned int InstanceID = 0;
//
//         //Sampled from depth buffer from the scene.
//         float CursorDepth = 0.0f;
//
//         //Coordinate of the screen space cursor inside the world. Calculated based on camera matrices and end depth buffer from the scene.
//         alignas(16) glm::vec3 WorldCursorCoordinate = {0.0f, 0.0f, 0.0f};
//
//     };
//
//     //"Picks" data from the screen and returns
//     class Tool_ScreenPicker: public Tool_Base
//     {
//     public:
//         void on_ui_icon_draw() override;
//
//     protected:
//
//         VkDescriptorSetLayout m_descriptorSetLayout = nullptr;
//         struct GPUData
//         {
//             VkBuffer buffer = nullptr;
//             VmaAllocation allocation = nullptr;
//             VkDescriptorSet descriptorSet = nullptr;
//             VkDescriptorPool descriptorPool = nullptr;
//         };
//         std::vector<GPUData> m_buffers;
//
//         ScreenPickData m_data;
//         ImVec2 m_uv = ImVec2(0.0f, 0.0f);
//         Tool_ScreenPickerOptionFlag m_PickerOptions = Tool_ScreenPickerOptionFlag::None;
//
//         VkPipeline m_sampleDataPipline = nullptr;
//         VkPipelineLayout m_sampleDataPipelineLayout = nullptr;
//
//         VkImage m_ModelIDsImage = nullptr;
//         VkImageView m_ModelIDsImageView = nullptr;
//         VmaAllocation m_ModelIDsImageAllocation = nullptr;
//
//         VkImage m_InstanceIDsImage = nullptr;
//         VkImageView m_InstanceIDsImageView = nullptr;
//         VmaAllocation m_InstanceIDsImageAllocation = nullptr;
//
//         VkImage m_pickDepthImage = nullptr;
//         VkImageView m_pickDepthImageView = nullptr;
//         VmaAllocation m_pickDepthImageAllocation = nullptr;
//
//
//     public:
//
//         inline const ScreenPickData& get_current_data() const {return m_data;}
//
//         void on_command_recording_compute(VkCommandBuffer commandBuffer) override;
//
//         void on_command_execution_finish() override;
//
//         int on_resize() override;
//
//         int create() override;
//         void destroy() override;
//     };
//
// }
