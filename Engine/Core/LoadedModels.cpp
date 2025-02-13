#include "LoadedModels.h"

using namespace Smoothie;

std::unordered_map<unsigned int, Smoothie::Model> LoadedModels::loadedModels;

Smoothie::Model* Smoothie::LoadedModels::getModel(unsigned int ModelID)
{
	if (auto search = loadedModels.find(ModelID); search != loadedModels.end())
	{
		return &loadedModels.at(ModelID);
	}
	return nullptr;
}

void Smoothie::LoadedModels::addModel(Smoothie::Model model)
{
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

void Smoothie::LoadedModels::removeModel(unsigned int ModelID)
{
	if (auto search = loadedModels.find(ModelID); search != loadedModels.end())
	{
		loadedModels.at(ModelID).destroy();
		loadedModels.erase(ModelID);
		return;
	}
	return;
}

void Smoothie::LoadedModels::destroyAllModels()
{
	for (auto&[ModelID, Model]: loadedModels)
	{
		Model.destroy();
	}
}
