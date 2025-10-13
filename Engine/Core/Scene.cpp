#include "Scene.h"
#include <filesystem>
#include <iostream>
#include <memory>

#include "ResourceManager/XML.h"
#include "Math/SmoothieMath.h"

#include "Core/Model.h"
#include "Core/Multithreading.h"
#include "Core/Shader.h"

#include "Effects/Skybox.h"
#include "Effects/DeferredPipeline.h"
#include "Effects/Deferred_Core.h"
#include "Core/SmoothieCore.h"

using namespace Smoothie::XML;

int Smoothie::Scene_Default::load_scene(const std::string& file)
{

	if (!std::filesystem::exists(file))
	{
		std::cout << "Scene file: " << file << " does not exist!" << std::endl;
		return 1;
	}

	Element rootElement;
	XMLError error;
	if (build_tree_from_xml_file(file, rootElement, error))
	{
		std::cout << "Failed to parse scene xml file!" << std::endl;
		return 1;
	}

	//Skybox
	const Element& skyboxElement = rootElement.getChild("skybox");
	read_skybox(skyboxElement);

	//Models
	const Element& modelsElement = rootElement.getChild("models");

	const std::vector<Element>& models = modelsElement.children;
	for (size_t i = 0; i < models.size(); i++)
	{
		__futures.push_back(std::async(std::launch::async, &Scene_Default::read_model, this, models[i]));
	}

	return 0;
}

void Smoothie::Scene_Default::clear_scene()
{
	vkDeviceWaitIdle(SmoothieCore::getDevice());
	for (const auto& future : __futures)
	{
		while (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
		{
			MultithreadSubmissions::submitGraphicsQueue();
		}
	}
	vkDeviceWaitIdle(SmoothieCore::getDevice());
	__futures.clear();
}


void Smoothie::Scene_Default::read_model(const Element& __model)
{
	const auto& matrix = __model.getChild("matrix");

	SmoothieMath::Matrix4x4 modelMatrix;
	if (__model.hasChild("position") && __model.hasChild("scale") && __model.hasChild("rotation"))
	{
		const auto& position = __model.getChild("position");
		const auto& scale = __model.getChild("scale");
		const auto& rotation = __model.getChild("rotation");
		modelMatrix.transformMatrix(position.getVector3(), rotation.getVector3(), scale.getVector3());
	}

	//Parse model file
	ModelFile __model_file;
	if (__model_file.create(__model.getChild("file").textContent) != 0)
	{
		std::cout << "Failed to parse model file: " << __model.getChild("file").textContent << std::endl;
		return;
	}

	std::shared_ptr<DeferredRendering::StandardModel> model = std::make_shared<DeferredRendering::StandardModel>();
	model->setModelFile(__model_file);
	model->setModelMatrix(modelMatrix);
	if (model->create() != 0)
	{
		std::cout << "Failed to create model " << __model_file.getModelFile() << std::endl;
		return;
	}
	auto* __pipeline = dynamic_cast<const DeferredRendering::StaticPipeline*>(model->getPipeline());
	__pipeline->add_to_rendering(model);
}

void Smoothie::Scene_Default::read_skybox(const XML::Element& skybox)
{
}
