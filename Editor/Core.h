// #pragma once
// #include <list>
// #include <memory>
// #include <map>
// #include <string>
//
// #include "Utilities/EditorWidgets.h"
// #include "Utilities/RenderingManager.h"
//
// namespace SmoothieEditor
// {
// 	enum Tools_Standard
// 	{
// 		TOOL_Default,
// 		TOOL_Camera,
// 	};
// 	using ToolsKey = int;
//
// 	class Core
// 	{
//
// 	protected:
// 		std::map<ToolsKey, std::shared_ptr<Tool_Base>> m_tools;
// 		std::unordered_map<std::string, Icon> m_Icons;
//
// 		Window_Editor editor_window;
// 		std::shared_ptr<RenderingManager> m_DrawingClass;
// 	public:
//
// 		virtual int create(std::shared_ptr<RenderingManager> rendering_manager);
// 		void on_command_record_time(VkCommandBuffer commandBuffer, VkDescriptorSet finalImage, VkImageView depthImage);
// 		void on_command_submit_time();
// 		virtual void destroy();
//
//
// 	};
// }
