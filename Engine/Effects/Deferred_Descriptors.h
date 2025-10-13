#pragma once
#include "Core/Buffer.h"
#include "Core/Descriptor.h"

namespace Smoothie 
{

	namespace DeferredRendering
	{

		struct DrawingDescriptorSetData
		{
			alignas(16) int time = 0;

			DrawingDescriptorSetData() = default;
		};

	
		class Deferred_Descriptor_Buffer: public BufferBase
		{
		public:
			int create() override;
			void resize_callback() override;
			void destroy() override;

			Deferred_Descriptor_Buffer() = default;
		};


		class Deferred_Descriptors: public DescriptorBase
		{
		protected:
			Deferred_Descriptor_Buffer buffer;

		public:
			int create() override;
			void destroy() override;
			void resize_callback() override;
			Deferred_Descriptors() = default;

			void update_descriptor_data(const DrawingDescriptorSetData& data);
		};



	}
}

