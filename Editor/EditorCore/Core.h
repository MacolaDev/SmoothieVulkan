#pragma once
#include <list>
#include <memory>

#include "EditorWidgets.h"
#include "RenderingManager.h"
#include "Icons.h"

namespace SmoothieEditor 
{
	//Contains global data for editor to work.
	struct Core
	{
		Icons icons;
		Window_Editor editor_window;
		std::shared_ptr<SmoothieEditor::RenderingManager> draw_manager = std::make_shared<SmoothieEditor::RenderingManager>();
	};

	Core& getCore();
}
