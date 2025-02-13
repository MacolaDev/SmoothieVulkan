#pragma once
#include <unordered_map>
#include "Core/Model.h"
#include <vector>

class Scene;
namespace Smoothie 
{
	//With this class it is possible to access all models that application has currently loaded and remove or add new ones.  
	class LoadedModels
	{
	public:
		
		//Searches a hash map and return a pointer to a location in memory that contains model with this ID.
		//If model with that ID does not exist, it returns nullptr.
		static Smoothie::Model* getModel(unsigned int ModelID);
		
		//Copies new model into hash map, making it accessable in hash map with it's own unique ID.
		//If model with same ID exists, it won't do anything.
		static void addModel(Smoothie::Model model);

		//Removes model with this ID. It also calls destroy() method on that model. 
		//If model with that ID does not exist, it does nothing.
		static void removeModel(unsigned int ModelID);

	private:
		static void destroyAllModels();
		static std::unordered_map<unsigned int, Smoothie::Model> loadedModels; 

		friend class Scene;
	};

}