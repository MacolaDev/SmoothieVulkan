#pragma once
#include <vulkan/vulkan.h>
#include "Core/Camera.h"
#include "Core/Constants.h"
#include "Core/Descriptor.h"
#include "Core/Buffer.h"
#include "vk_mem_alloc.h"

namespace Smoothie
{

	class CameraDescriptorBuffer : public BufferBase
	{

	public:
		int create() override;
		void resize_callback() override;
		void destroy() override;
	};

	class CameraDescriptorSet : public DescriptorBase
	{
		void resize_callback() override;
		CameraDescriptorBuffer buffer;
	public:
		int create() override;
		void update_camera_data(const CameraUniformBufferData& data);
		void destroy() override;

	};
}