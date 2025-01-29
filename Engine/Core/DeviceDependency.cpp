#include "DeviceDependency.h"
#include <iostream>
#include <array>

static const std::array<VkFormat, 2> depthFormats =
{
	VK_FORMAT_D32_SFLOAT,
	VK_FORMAT_D16_UNORM
};
VkFormat DeviceDependencies::depthFormat = VK_FORMAT_UNDEFINED;

VkFormat DeviceDependencies::getSupportedDepthFormat()
{
	return depthFormat;
}

void DeviceDependencies::getDendepndenciesFromDevice(VkPhysicalDevice physicalDevice)
{
	//Depth format selection
	for (int i = 0; i < depthFormats.size(); i++) 
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, depthFormats[i], &formatProperties);
		if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) 
		{
			depthFormat = depthFormats[i];
			break;
		}
	}
	if (depthFormat == VK_FORMAT_UNDEFINED)
	{
		std::cout << __FUNCTION__": Currently selected GPU does not support any valid depth format!" << std::endl;
	}
}
