#pragma once
#include <string>
#include <mutex>
#include <vector>
#include <future>
#include <ResourceManager/XML.h>

namespace Smoothie 
{
	class Scene_Default 
	{
	protected:
		std::vector<std::future<void>> __futures; //Contains model futures for multithreaded loading of stuff in engine
		std::string scene_file;
	public:

		//This method reads string files and it is supposed to invoke all other functions for various objects to load in multithreaded fashion
		virtual int load_scene(const std::string& file);

		//This method should 
		virtual void clear_scene();
		
		virtual void read_model(const XML::Element& model);

		virtual void read_skybox(const XML::Element& skybox);
	};


}