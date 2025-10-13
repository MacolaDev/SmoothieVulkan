#pragma once
#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <memory>

#include <Math/SmoothieMath.h>
#include "ResourceManager/XML.h"

#include "Core/Constants.h"
#include "Core/Descriptor.h"
#include "Core/Pipeline.h"

namespace Smoothie 
{
	class Model_Base 
	{
		unsigned int ModelID = 0;
	protected:
		inline void setID(unsigned int _newId) { ModelID = _newId; }
		std::shared_ptr<Pipeline_Base> pipeline;

	public:
		virtual int create() = 0;
		inline virtual int update() { return 0; };
		virtual void destroy() = 0;
		
		inline unsigned int getID() const { return ModelID; }
		inline void setPipeline(std::shared_ptr<Pipeline_Base>& pipeline) { this->pipeline = pipeline; }
		inline const Pipeline_Base* getPipeline() const { return pipeline.get(); }
		Model_Base() = default;
	};

	//Class used for parsing model files.
	class ModelFile
	{
	protected:

		std::string shader_file;
		std::string model_file;
		std::string geometry_file;
		SmoothieMath::Vector3 bbMin;
		SmoothieMath::Vector3 bbMax;
		std::vector<XML::Element> model_properties;

	public:

		//This method parses the model file. 
		virtual int create(const std::string& model_file);

		//Model can be saved as xml file. This methods builds the tree root.
		virtual int build_as_tree(XML::Element& rootElement);

		inline const std::string& getShaderFile() const { return shader_file; }
		inline const std::string& getModelFile() const { return model_file; }
		inline const std::string& getGeometryFile() const { return geometry_file; }

		inline const std::vector<XML::Element>& getMaterialProperties() const { return model_properties; }

	};

}
