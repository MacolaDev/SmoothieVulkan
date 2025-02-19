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
#include "Core/Multithreading.h"

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
	DeferredPipeline::create(SmoothieCore::SCR_WIDTH, SmoothieCore::SCR_HEIGHT);
	std::vector<Element> models = xmlData.getElement("models").children;
	
	for (size_t i = 0; i < models.size(); i++)
	{
		modelFutures.push_back(std::async(std::launch::async, loadModelsAsync, models[i]));
	}
	
	Skybox::create(PBRCubemaps::HDRCubemap);
}

static void waitForAllFuturesToFinish(const std::vector<std::future<void>>& futures) 
{
	for (const auto& future : futures)
	{
		while (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
		{
			MultithreadSubmissions::submitGraphicsQueue();
		}
	}
}

void Scene::destroy()
{
	vkQueueWaitIdle(SmoothieCore::getGraphicsQueue());
	
	waitForAllFuturesToFinish(modelFutures);
	modelFutures.clear();

	vkQueueWaitIdle(SmoothieCore::getGraphicsQueue());

	Skybox::destroy();
	Smoothie::LoadedModels::destroyAllModels();
	PBRCubemaps::destroy();
	DeferredPipeline::destroy();
}

void Scene::update(unsigned int width, unsigned int height)
{
	vkDeviceWaitIdle(SmoothieCore::getDevice());
	DeferredPipeline::update(width, height);
}
