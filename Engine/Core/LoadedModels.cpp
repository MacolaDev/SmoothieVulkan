#include "LoadedModels.h"

using namespace Smoothie;

std::unordered_map<unsigned int, Smoothie::Model> LoadedModels::loadedModels;

static std::mutex getModelMutex;
Smoothie::Model* Smoothie::LoadedModels::getModel(unsigned int ModelID)
{
	std::lock_guard<std::mutex> lock(getModelMutex);
	if (auto search = loadedModels.find(ModelID); search != loadedModels.end())
	{
		return &loadedModels.at(ModelID);
	}
	return nullptr;
}

static std::mutex addModelMutex;
void Smoothie::LoadedModels::addModel(Smoothie::Model model)
{
	std::lock_guard<std::mutex> lock(addModelMutex);
	const unsigned int modelID = model.getModelID();
	if (auto search = loadedModels.find(modelID); search != loadedModels.end())
	{
		return;
	}
	else
	{
		loadedModels[modelID] = model;
	}
	return;
}

std::mutex removeModelMutex;
void Smoothie::LoadedModels::removeModel(unsigned int ModelID)
{
	std::lock_guard<std::mutex> lock(removeModelMutex);
	if (auto search = loadedModels.find(ModelID); search != loadedModels.end())
	{
		loadedModels.erase(ModelID);
	}
}

static std::mutex destroyModelMutex;
void Smoothie::LoadedModels::destroyAllModels()
{
	std::lock_guard<std::mutex> lock(destroyModelMutex);
	for (auto&[ModelID, Model]: loadedModels)
	{
		Model.destroy();
	}
}
