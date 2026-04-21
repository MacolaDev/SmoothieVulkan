#include "Model.h"
#include "ResourceManager/XML.h"

#include "Core/SmoothieCore.h"
#include "Core/RenderPass.h"
#include "Core/CameraDescriptor.h"
#include "Core/Pipeline.h"


#include <limits>
#include <random>
#include <mutex>
#include <iostream>

using namespace Smoothie;
using namespace SmoothieMath;
using namespace Smoothie::XML;

int Smoothie::ModelFile::create()
{
	Element _rootElement;
	XMLError _error;
	if (build_tree_from_xml_file(m_modelFile, _rootElement, _error) != 0)
	{
		std::cout << "Failed to parse: " << m_modelFile << std::endl;
		return 1;
	}

	if (_rootElement.hasChild("geometryFile") != true)
	{
		std::cout << "No geometry file element in file: " << m_modelFile << std::endl;
		return 1;
	}
	m_geometryFile = _rootElement.getChild("geometryFile").textContent;
	
	//Shader
	if (_rootElement.hasChild("shader") != true)
	{
		std::cout << "No shader file element in file: " << m_modelFile << std::endl;
		return 1;
	}
	m_shaderFile = _rootElement.getChild("shader").textContent;

	//properties
	m_modelProperties = _rootElement.getChild("property").children;
	
	//Bounding box 
	const Element& _boundingBoxElement = _rootElement.getChild("BoundingBox");
	if (_boundingBoxElement.hasChild("min"))
	{
		m_bbMin = _boundingBoxElement.getChild("min").getVector3();
	}
	else
	{
		m_bbMin = { 0.0f, 0.0f, 0.0f };
	}

	if (_boundingBoxElement.hasChild("max"))
	{
		m_bbMax = _boundingBoxElement.getChild("max").getVector3();
	}
	else
	{
		m_bbMax = { 1.0f, 1.0f, 1.0f };
	}

	return 0;
}

int Smoothie::ModelFile::build_as_tree(XML::Element& rootElement)
{
	return 0;
}
