#pragma once
#include <string>
#include <mutex>
#include <vector>
#include <future>

class Scene
{
public:
	Scene(const std::string& file);
	Scene() = default;

	void destroy();
private:
	std::vector<std::future<void>> modelFutures;
};

