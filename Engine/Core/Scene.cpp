#include "Scene.h"
#include <filesystem>
#include <iostream>
#include "ResourceManager/ParseXML.h"
#include "Effects/BRDF.h"
#include "Core/DeferredPipeline.h"
#define _SMOOTHIE_ENGINE
#include "Core/SmoothieCore.h"
#include "Core/Model.h"
#include "Core/LoadedModels.h"
#include "Math/SmoothieMath.h"
#include "Effects/Skybox.h"

static std::mutex ModelMutex;
static void loadModelsAsync(Element modelElement)
{
	Element* matrix = modelElement.getChild("matrix");
	Element* position = modelElement.getChild("position");
	Element* scale = modelElement.getChild("scale");
	Element* rotation = modelElement.getChild("rotation");

	SmoothieMath::Matrix4x4 modelMatrix;
	if (position && rotation && scale)
	{
		modelMatrix.transformMatrix(position->getVector3(), rotation->getVector3(), scale->getVector3());
	}

	auto model = Smoothie::Model(modelElement.getChild("file")->textContent, modelMatrix);

	std::lock_guard<std::mutex> lock(ModelMutex);
	
	Smoothie::LoadedModels::addModel(model);
	model.addToRendering();
}

Scene::Scene(const std::string& file)
{
	if (!std::filesystem::exists(file)) 
	{
		std::cout << __FUNCTION__": Scene file: " << file << " is not a valid file!" << std::endl;
		return;
	}

	auto xmlData = ParseXML(file);
	PBRCubemaps::create(xmlData.getElement("skybox").textContent);
	DeferredPipeline::create(1280, 720);
	std::vector<Element> models = xmlData.getElement("models").children;
	
	for (size_t i = 0; i < models.size(); i++)
	{
		loadModelsAsync(models[i]);
		//modelFutures.push_back(std::async(std::launch::async, loadModelsAsync, models[i]));
	}
	
	Skybox::create(PBRCubemaps::HDRCubemap);
}

void Scene::destroy()
{
	vkQueueWaitIdle(SmoothieCore::getGraphicsQueue());

	Skybox::destroy();
	Smoothie::LoadedModels::destroyAllModels();
	PBRCubemaps::destroy();
	DeferredPipeline::destroy();
}
