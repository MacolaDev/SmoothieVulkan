#pragma once
#include <unordered_map>
#include "Core/Model.h"

struct ModelPassBase
{
	ModelPassBase() = default;
	virtual void addToPass(unsigned int ModelID) {};
	virtual void removeFromPass(unsigned int ModelID) {};
};

struct ModelPassGBUFFER: public ModelPassBase
{

};
