#pragma once
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include <Math/SmoothieMath.h>
#include "ResourceManager/XML.h"

namespace Smoothie 
{

	class Model_Base 
	{
	public:
		virtual int create() = 0;
		virtual int update() = 0;
		virtual void destroy() = 0;
		virtual ~Model_Base() = default;
	};

	//Class used for parsing model files.
	class ModelFile
	{
	protected:

		std::string m_shaderFile;
		std::string m_modelFile;
		std::string m_geometryFile;
		SmoothieMath::Vector3 m_bbMin;
		SmoothieMath::Vector3 m_bbMax;
		std::vector<XML::Element> m_modelProperties;

	public:

		inline void set_model_file(const std::string& file) { m_modelFile = file; }

		//This method parses the model file. 
		virtual int create();

		//Model can be saved as xml file. This methods builds the tree root.
		virtual int build_as_tree(XML::Element& rootElement);

		inline const std::string& getShaderFile() const { return m_shaderFile; }
		inline const std::string& getModelFile() const { return m_modelFile; }
		inline const std::string& getGeometryFile() const { return m_geometryFile; }

		inline const std::vector<XML::Element>& getMaterialProperties() const { return m_modelProperties; }

	};

}
