#include "Scene.h"

#include "ResourceManager/XML.h"
#include "Math/SmoothieMath.h"

#include "Core/Model.h"
#include "Core/Multithreading.h"
#include "Core/Shader.h"

#include "Effects/Skybox.h"
#include "../Effects/Pipelines/Standard.h"
#include "Effects/Deferred_Core.h"
#include "Core/SmoothieCore.h"

using namespace Smoothie::XML;

// int Smoothie::Scene_Base::load_scene(const std::string& file)
// {
//
// 	if (!std::filesystem::exists(file))
// 	{
// 		std::cout << "Scene file: " << file << " does not exist!" << std::endl;
// 		return 1;
// 	}
//
// 	Element rootElement;
// 	XMLError error;
// 	if (build_tree_from_xml_file(file, rootElement, error))
// 	{
// 		std::cout << "Failed to parse scene xml file!" << std::endl;
// 		return 1;
// 	}
//
// 	//Skybox
// 	const Element& skyboxElement = rootElement.getChild("skybox");
// 	//read_skybox(skyboxElement);
//
// 	//Models
// 	const Element& modelsElement = rootElement.getChild("models");
//
// 	const std::vector<Element>& models = modelsElement.children;
// 	for (size_t i = 0; i < models.size(); i++)
// 	{
// 		m_Futures.push_back(std::async(std::launch::async, &Scene_Base::read_model, this, models[i]));
// 	}
//
// 	return 0;
// }


// void Smoothie::Scene_Base::_clear_scene()
// {
// 	vkDeviceWaitIdle(SmoothieCore::getDevice());
// 	for (const auto& future : m_Futures)
// 	{
// 		// while (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
// 		// {
// 		// 	MultithreadSubmissions::submitGraphicsQueue();
// 		// }
// 	}
// 	vkDeviceWaitIdle(SmoothieCore::getDevice());
// 	m_Futures.clear();
// }


// void Smoothie::Scene_Base::read_model(const Element& _model_element)
// {
// 	const auto& matrix = _model_element.getChild("matrix");
//
// 	SmoothieMath::Matrix4x4 _modelMatrix;
// 	if (_model_element.hasChild("position") && _model_element.hasChild("scale") && _model_element.hasChild("rotation"))
// 	{
// 		const auto& position = _model_element.getChild("position");
// 		const auto& scale = _model_element.getChild("scale");
// 		const auto& rotation = _model_element.getChild("rotation");
// 		_modelMatrix.transformMatrix(position.getVector3(), rotation.getVector3(), scale.getVector3());
// 	}
//
//
// 	DeferredRendering::Standard_Model _model;
// 	_model.add_model_matrix(_modelMatrix);
// 	_model.setModelFile(_model_element.getChild("file").textContent);
// 	if (_model.create() != 0)
// 	{
// 		std::cout << "Failed to create model " << _model.getModelFile() << std::endl;
// 		return;
// 	}
// }

