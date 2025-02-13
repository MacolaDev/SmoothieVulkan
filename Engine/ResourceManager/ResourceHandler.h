#pragma once
#include <unordered_map>
#include <string>
#include <iostream>
#include <mutex>

template<class Resource> struct SafeResourceCount
{
	unsigned int ResourceCout = 0;
	Resource resource;
};

template<class Resource> class ResourceHandler
{
public:
	Resource* getResourse(const std::string& filepath) const;
	void setResource(const std::string& filepath, Resource resouce);
	bool isAlreadyLoaded(const std::string& filepath) const;
	unsigned int getReferenceCout(const std::string& filepath) const;
	void decreaseReferenceCount(const std::string& filepath);
	void increaseReferenceCount(const std::string& filepath);
	void removeResource(const std::string& filepath);

private:
	static std::unordered_map<std::string, SafeResourceCount<Resource>> resources;
};

template<class Resource>std::unordered_map<std::string, SafeResourceCount<Resource>>ResourceHandler<Resource>::resources;

template<class Resource>
inline Resource* ResourceHandler<Resource>::getResourse(const std::string& filepath) const
{
	if (auto search = resources.find(filepath); search != resources.end())
	{
		return &resources[filepath].resource;
	}
	return nullptr;
}

static std::mutex writingMutex;
template<class Resource>
inline void ResourceHandler<Resource>::setResource(const std::string& filepath, Resource resouce)
{
	
	std::lock_guard<std::mutex> lock(writingMutex);

	if (auto search = resources.find(filepath); search != resources.end())
	{

#ifdef _DEBUG
		std::cout << "Resource file already loaded: " + filepath << std::endl;
#endif // _DEBUG
		resources[filepath].ResourceCout += 1;
	}
	else
	{

#ifdef _DEBUG
		std::cout << "Loading resorce file: " + filepath << std::endl;
#endif // _DEBUG

		resources[filepath].resource = resouce;
		resources[filepath].ResourceCout += 1;

	}

	
}

template<class Resource>
inline bool ResourceHandler<Resource>::isAlreadyLoaded(const std::string& resourceName) const
{
	if (auto search = resources.find(resourceName); search != resources.end())
	{
		return true;
	}
	return false;
}

template<class Resource>
inline unsigned int ResourceHandler<Resource>::getReferenceCout(const std::string& filepath) const
{

	if (auto search = resources.find(filepath); search != resources.end())
	{
		return resources.at(filepath).ResourceCout;
	}
	return 0;
}

template<class Resource>
inline void ResourceHandler<Resource>::decreaseReferenceCount(const std::string& filepath)
{

	if (auto search = resources.find(filepath); search != resources.end())
	{
		resources.at(filepath).ResourceCout -= 1;
	}

}

template<class Resource>
inline void ResourceHandler<Resource>::increaseReferenceCount(const std::string& filepath)
{
	if (auto search = resources.find(filepath); search != resources.end())
	{
		resources.at(filepath).ResourceCout += 1;
	}
}

template<class Resource>
inline void ResourceHandler<Resource>::removeResource(const std::string& filepath)
{
	if (auto search = resources.find(filepath); search != resources.end())
	{

		resources.erase(filepath);

#ifdef _DEBUG
			std::cout << "Resource file is getting deleted: " + filepath << std::endl;
#endif // _DEBUG

	}
}
