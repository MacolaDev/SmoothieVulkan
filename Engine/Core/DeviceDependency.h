#pragma once
#include <vulkan/vulkan.h>

class SmoothieCore;
class DeviceDependencies 
{
	friend class SmoothieCore;
	static void getDendepndenciesFromDevice(VkPhysicalDevice physicalDevice);
public:
	static VkFormat getSupportedDepthFormat();

private:
	static VkFormat depthFormat;
};

