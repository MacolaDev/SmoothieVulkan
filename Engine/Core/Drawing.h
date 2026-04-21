#pragma once
#include <vulkan/vulkan.h>

#include <memory>

#include "Editor_Core.h"

namespace Smoothie
{
	//Base class for managing drawing actions to create a single frame
	class Drawing_Base
	{
		std::shared_ptr<Smoothie::Editor_Core> m_editor_core = nullptr;
	public:

		virtual int create() = 0;
		virtual void draw(VkCommandBuffer commandBuffer, unsigned int frame) const = 0;
		
		//Gets called whenever window gets resized
		virtual int resize_callback() = 0;
		
		virtual void destroy() = 0;

#ifdef SMOOTHIE_VULKAN_EDITOR
		inline void	set_editor_core(std::shared_ptr<Smoothie::Editor_Core> editor_core) { m_editor_core = editor_core; }
#endif

		inline Smoothie::Editor_Core* get_editor_core() const {return m_editor_core.get();};
		inline std::shared_ptr<Smoothie::Editor_Core>& get_editor_corePtr() {return m_editor_core;}

		virtual ~Drawing_Base() = default;
	};
}

