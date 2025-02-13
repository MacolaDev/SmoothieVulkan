#pragma once
#include <vector>
#include "Math/SmoothieMath.h"
#include <vulkan/vulkan.h>
#include "Core/VMA.h"


class UniformBuffers
{
		
	VkBuffer uniformBuffer = nullptr;
	VmaAllocation uniformBufferAllocation = nullptr;

	VkDescriptorPool descriptorPool = nullptr;
	VkDescriptorSetLayout descriptorSetLayout = nullptr;
	VkDescriptorSet descriptorSet = nullptr;

	std::vector<unsigned char> bufferData;

	std::vector<void*> elementDataPointers;
	std::vector<unsigned int> bufferElementSizes;

public:
	
	UniformBuffers() = default;
	void createBufferFromData();
	void destroy();

	void addDataInt(int* data);
	void addDataFloat(float* data);
	void addDataVector3(SmoothieMath::Vector3* data);
	void addDataMatrix4x4(SmoothieMath::Matrix4x4* data);
	void addDataBool(bool* data);

	VkDescriptorSetLayout getDescriptorSetLayout() const;
	VkDescriptorSet getDescriptorSet() const;
};


struct StandardUnifromBuffers 
{
	static UniformBuffers buffer2;
};