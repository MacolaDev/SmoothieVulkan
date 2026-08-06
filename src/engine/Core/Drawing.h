#pragma once
#include "../Common.h"
#include "Camera.h"

namespace Smoothie
{
	//Base class for managing drawing actions to create a single frame
	class Drawing_Base
	{
	public:

		virtual int create() = 0;
		virtual void draw(VkCommandBuffer commandBuffer, unsigned int frame) const = 0;
		
		//Gets called whenever window gets resized
		virtual int resize_callback() = 0;
		
		virtual void destroy() = 0;

	    virtual void update_camera(const Camera& camera) {};

		virtual ~Drawing_Base() = default;
	};
}

