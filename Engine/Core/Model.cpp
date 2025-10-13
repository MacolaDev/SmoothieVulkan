#include "Model.h"
#include "ResourceManager/XML.h"

#include "Core/SmoothieCore.h"
#include "Core/RenderPass.h"
#include "Core/CameraDescriptor.h"
#include "Core/Pipeline.h"
#include "Core/Image.h"


#include <limits>
#include <random>
#include <mutex>

using namespace Smoothie;
using namespace SmoothieMath;
using namespace Smoothie::XML;

int Smoothie::ModelFile::create(const std::string& _model_file)
{
	this->model_file = _model_file;

	Element rootElement;
	XMLError error;
	if (build_tree_from_xml_file(model_file, rootElement, error) != 0)
	{
		std::cout << "Failed to parse: " << model_file << std::endl;
		return 1;
	}

	if (rootElement.hasChild("geometryFile") != true)
	{
		std::cout << "No geometry file element in file: " << model_file << std::endl;
		return 1;
	}
	geometry_file = rootElement.getChild("geometryFile").textContent;
	
	//Shader
	if (rootElement.hasChild("shader") != true)
	{
		std::cout << "No shader file element in file: " << model_file << std::endl;
		return 1;
	}
	shader_file = rootElement.getChild("shader").textContent;

	//properties
	model_properties = rootElement.getChild("property").children;
	
	//Bounding box 
	const Element& boundingBoxElement = rootElement.getChild("BoundingBox");
	if (boundingBoxElement.hasChild("min"))
	{
		bbMin = boundingBoxElement.getChild("min").getVector3();
	}
	else
	{
		bbMin = { 0.0f, 0.0f, 0.0f };
	}

	if (boundingBoxElement.hasChild("max"))
	{
		bbMax = boundingBoxElement.getChild("max").getVector3();
	}
	else
	{
		bbMax = { 1.0f, 1.0f, 1.0f };
	}

	return 0;
}

int Smoothie::ModelFile::build_as_tree(XML::Element& rootElement)
{
	return 0;
}
