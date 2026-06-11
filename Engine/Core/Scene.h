#pragma once
#include "Core/Common.h"
#include <list>
#include <filesystem>
#include <unordered_set>

#include "Core/Model.h"

namespace Smoothie 
{

	class LoadableObject_Base
	{
	protected:
		std::filesystem::file_time_type m_WriteTime;
	public:

		inline const std::filesystem::file_time_type& get_WriteTime() const { return m_WriteTime; }

		//Implementation must be thread safe.
		virtual int recreate() = 0;
		virtual int create() = 0;
		virtual void destroy() = 0;

		virtual ~LoadableObject_Base() = default;

	};

	class Scene_Base
	{
		std::unordered_map<std::string, std::shared_ptr<LoadableObject_Base>> m_LoadedObjects;
		std::mutex m_LoadedObjects_Mutex;
	protected:
		std::string m_SceneFile;
	public:

		//This function is thread safe.
		inline void add_Object(const std::string& file, const std::shared_ptr<LoadableObject_Base>& object)
		{
			std::lock_guard<std::mutex> _lock(m_LoadedObjects_Mutex);
			m_LoadedObjects.insert(std::make_pair(file, object));
		}

		//This function is thread safe.
		inline void remove_Object(const std::string& file)
		{
			std::lock_guard<std::mutex> _lock(m_LoadedObjects_Mutex);
			m_LoadedObjects.erase(file);
		}

		//This function is thread safe.
		inline std::shared_ptr<LoadableObject_Base> get_Object(const std::string& file)
		{
			std::lock_guard<std::mutex> _lock(m_LoadedObjects_Mutex);
			return m_LoadedObjects.find(file) != m_LoadedObjects.end() ? m_LoadedObjects.at(file) : nullptr;
		}

		inline const std::string& get_SceneFile()  const { return m_SceneFile; }

		//This method reads string files and it is supposed to invoke all other functions for various objects to load in multithreaded fashion
		virtual int load_scene(const std::string& file) = 0;

		virtual int save_scene(const std::string& file) {return 0;}

		 inline void _clear_scene()
		{
			std::lock_guard<std::mutex> _lock(m_LoadedObjects_Mutex);
			for (auto& [path, obj]: m_LoadedObjects)
			{
				obj->destroy();
			}
			m_LoadedObjects.clear();
		}
		virtual ~Scene_Base() = default;
	};


}