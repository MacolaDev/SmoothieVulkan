#pragma once
#include <vulkan/vulkan.h>
namespace Smoothie
{
	//Base class for all screen space effects
	class Effect_Base
	{
	protected:


	public:
		virtual int create() = 0;
		virtual void draw(VkCommandBuffer commandBuffer, VkDescriptorSet drawClassSet, unsigned int FrameID) const = 0;
		virtual void destroy() = 0;

		virtual int resize_callback() = 0;
		Effect_Base() = default;
	};
}
