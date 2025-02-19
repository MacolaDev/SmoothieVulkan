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
	void update(unsigned int width, unsigned int height);
private:
	std::vector<std::future<void>> modelFutures;
};

