#pragma once
#include <unordered_map>
#include <string>
#include <iostream>
#include <mutex>

template<class Resource> struct SafeResourceCount
{
	unsigned int ResourceCout = 0;
	Resource resource;
	SafeResourceCount() = default;
};

template<class Resource> class ResourceHandler
{
	static std::unordered_map<std::string, SafeResourceCount<Resource>> s_resourcesMap;
	static std::mutex s_resourcesMapMutex;
protected:

	virtual int create(const std::string& filepath) = 0;
	virtual void destroy() = 0;

public:
	/*Resource* getResourse(const std::string& filepath) const;
	void setResource(const std::string& filepath, Resource resouce);
	bool isAlreadyLoaded(const std::string& filepath) const;
	unsigned int getReferenceCout(const std::string& filepath) const;
	void decreaseReferenceCount(const std::string& filepath);
	void increaseReferenceCount(const std::string& filepath);
	void removeResource(const std::string& filepath);*/

	static const Resource& getResource(const std::string& filepath, int& error);
	static void removeResource(const std::string& filepath);

	ResourceHandler() = default;
};


template<class Resource>std::unordered_map<std::string, SafeResourceCount<Resource>>ResourceHandler<Resource>::s_resourcesMap;
template<class Resource>std::mutex ResourceHandler<Resource>::s_resourcesMapMutex;

//template<class Resource>
//inline Resource* ResourceHandler<Resource>::getResourse(const std::string& filepath) const
//{
//	std::lock_guard<std::mutex> lock(mapMutex);
//	if (auto search = resources.find(filepath); search != resources.end())
//	{
//		return &resources[filepath].resource;
//	}
//	return nullptr;
//}
//
//template<class Resource>
//inline void ResourceHandler<Resource>::setResource(const std::string& filepath, Resource resouce)
//{
//	
//	std::lock_guard<std::mutex> lock(mapMutex);
//
//	if (auto search = resources.find(filepath); search != resources.end())
//	{
//
//#ifdef _DEBUG
//		std::cout << "Resource file already loaded: " + filepath << std::endl;
//#endif // _DEBUG
//		resources[filepath].ResourceCout += 1;
//	}
//	else
//	{
//
//#ifdef _DEBUG
//		std::cout << "Loading resorce file: " + filepath << std::endl;
//#endif // _DEBUG
//
//		resources[filepath].resource = resouce;
//		resources[filepath].ResourceCout += 1;
//
//	}
//
//	
//}
//
//template<class Resource>
//inline bool ResourceHandler<Resource>::isAlreadyLoaded(const std::string& resourceName) const
//{
//	std::lock_guard<std::mutex> lock(mapMutex);
//	if (auto search = resources.find(resourceName); search != resources.end())
//	{
//		return true;
//	}
//	return false;
//}
//
//template<class Resource>
//inline unsigned int ResourceHandler<Resource>::getReferenceCout(const std::string& filepath) const
//{
//	std::lock_guard<std::mutex> lock(mapMutex);
//	if (auto search = resources.find(filepath); search != resources.end())
//	{
//		return resources.at(filepath).ResourceCout;
//	}
//	return 0;
//}
//
//template<class Resource>
//inline void ResourceHandler<Resource>::decreaseReferenceCount(const std::string& filepath)
//{
//	std::lock_guard<std::mutex> lock(mapMutex);
//	if (auto search = resources.find(filepath); search != resources.end())
//	{
//		resources.at(filepath).ResourceCout -= 1;
//	}
//
//}
//
//template<class Resource>
//inline void ResourceHandler<Resource>::increaseReferenceCount(const std::string& filepath)
//{
//	std::lock_guard<std::mutex> lock(mapMutex);
//	if (auto search = resources.find(filepath); search != resources.end())
//	{
//		resources.at(filepath).ResourceCout += 1;
//	}
//}
//
//template<class Resource>
//inline void ResourceHandler<Resource>::removeResource(const std::string& filepath)
//{
//	std::lock_guard<std::mutex> lock(mapMutex);
//	if (auto search = resources.find(filepath); search != resources.end())
//	{
//
//		resources.erase(filepath);
//
//#ifdef _DEBUG
//			std::cout << "Resource file is getting deleted: " + filepath << std::endl;
//#endif // _DEBUG
//
//	}
//}

template<class Resource>
inline const Resource& ResourceHandler<Resource>::getResource(const std::string& filepath, int& error)
{
	std::lock_guard<std::mutex> lock(s_resourcesMapMutex);
	if (s_resourcesMap.find(filepath) != s_resourcesMap.end())
	{
		error = 0;
		s_resourcesMap[filepath].ResourceCout += 1;
		return s_resourcesMap[filepath].resource;
	}
	else
	{
		SafeResourceCount<Resource>& __rss = s_resourcesMap[filepath];
		error = __rss.resource.create(filepath);
		if (error != 0)
		{
			s_resourcesMap.erase(filepath);
		}
		s_resourcesMap[filepath].ResourceCout += 1;
		return s_resourcesMap[filepath].resource;
	}
}

template<class Resource>
inline void ResourceHandler<Resource>::removeResource(const std::string& filepath)
{
	std::lock_guard<std::mutex> lock(s_resourcesMapMutex);
	if (s_resourcesMap.find(filepath) == s_resourcesMap.end()) return;
	
	SafeResourceCount<Resource>& __rss = s_resourcesMap[filepath];
	__rss.ResourceCout -= 1;
	if (__rss.ResourceCout <= 0)
	{
		__rss.resource.destroy();
		s_resourcesMap.erase(filepath);
		return;
	}
	return;
}
