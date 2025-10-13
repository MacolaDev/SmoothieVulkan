#pragma once
#include <Core/Buffer.h>
#include "ResourceManager/XML.h"
#include "Math/SmoothieMath.h"
#include "Core/Shader.h"

namespace Smoothie 
{
	namespace DeferredRendering 
	{

		class UniformBuffer_Base: public Smoothie::BufferBase
		{
		public: 

			
			struct Reserved_Values
			{
				unsigned int ModelID = 0;
				SmoothieMath::Matrix4x4 modelMatrix;
				Reserved_Values() = default;
			};

			virtual int create() override;
			virtual int create(
				const std::unordered_map<std::string, Smoothie::ShaderObjectReflection>& _reflections, 
				const std::vector<XML::Element>& _properties, 
				const Reserved_Values& _reserved_values) = 0;
			virtual void destroy() = 0;


		};

		//Data in this buffer is read-only. 
		class UniformBuffer_Static: public UniformBuffer_Base
		{
		public:

			int create(
				const std::unordered_map<std::string, Smoothie::ShaderObjectReflection>& _reflections,
				const std::vector<XML::Element>& _properties,
				const Reserved_Values& _reserved_values) override;
			
			void destroy() override;
			void resize_callback() override;
		};
	
		
	}

}

