#include "UniformBuffers.h"

#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"

static unsigned char paddedBuffer[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

UniformBuffers StandardUnifromBuffers::buffer1;
UniformBuffers StandardUnifromBuffers::buffer2;

void UniformBuffers::createBufferFromData()
{
	//********************** Padding existing buffer to 16 bytes *********************//
	const unsigned int bufferSize = static_cast<unsigned int>(bufferData.size());
	if ((bufferSize % 16) != 0)
	{
		for (unsigned int i = 0; i < 16 - (bufferSize % 16); i++)
		{
			bufferData.push_back(paddedBuffer[0]);
		}

		bufferElementSizes.push_back(16 - (bufferSize % 16));
		elementDataPointers.push_back(&paddedBuffer[0]);
	}


	//********************** Creating uniform buffer *********************************//
	VkBufferCreateInfo uniformBufferCreateInfo{};
	uniformBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	uniformBufferCreateInfo.size = bufferData.size();
	uniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	vmaCreateBuffer(VMA::getAllocator(), &uniformBufferCreateInfo, &allocInfo, &uniformBuffer, &uniformBufferAllocation, nullptr);
	vmaCopyMemoryToAllocation(VMA::getAllocator(), bufferData.data(), uniformBufferAllocation, 0, bufferData.size());

	
	//********************** Creating descriptor pool ********************************//
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<unsigned int>(elementDataPointers.size());

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;
	vkCreateDescriptorPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &descriptorPool);

	//********************** Creating descriptor set layouts *************************//
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0; 
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;
	vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &layoutInfo, nullptr, &descriptorSetLayout);


	//********************** Creating descriptor set *********************************//
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
	vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &descriptorSet);


	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = uniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr;

	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);

}

void UniformBuffers::destroy() 
{
	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), descriptorSetLayout, nullptr);
	descriptorSetLayout = nullptr;
	vkDestroyDescriptorPool(SmoothieCore::getDevice(), descriptorPool, nullptr);
	descriptorPool = nullptr;
	
	vmaDestroyBuffer(VMA::getAllocator(), uniformBuffer, uniformBufferAllocation);
	uniformBuffer = nullptr;
	uniformBufferAllocation = nullptr;
}

void UniformBuffers::addDataInt(int* data)
{
	elementDataPointers.push_back(data);
	bufferElementSizes.push_back(sizeof(int));
	
	unsigned char* charPtr = reinterpret_cast<unsigned char*>(data);
	for (unsigned int i = 0; i < sizeof(int); i++)
	{
		const unsigned char byte = charPtr[i];
		bufferData.push_back(byte);
	}
}

void UniformBuffers::addDataFloat(float* data)
{
	elementDataPointers.push_back(data);
	bufferElementSizes.push_back(sizeof(float));

	unsigned char* charPtr = reinterpret_cast<unsigned char*>(data);
	for (unsigned int i = 0; i < sizeof(float); i++)
	{
		const unsigned char byte = charPtr[i];
		bufferData.push_back(byte);
	}
}

void UniformBuffers::addDataVector3(SmoothieMath::Vector3* data)
{
	elementDataPointers.push_back(data);
	bufferElementSizes.push_back(sizeof(SmoothieMath::Vector3));

	unsigned char* charPtr = reinterpret_cast<unsigned char*>(data);
	for (unsigned int i = 0; i < sizeof(SmoothieMath::Vector3); i++)
	{
		const unsigned char byte = charPtr[i];
		bufferData.push_back(byte);
	}

	//Padding to 16 bytes
	elementDataPointers.push_back(&paddedBuffer[0]);
	bufferElementSizes.push_back(sizeof(float));

	charPtr = reinterpret_cast<unsigned char*>(paddedBuffer);
	for (unsigned int i = 0; i < sizeof(float); i++)
	{
		const unsigned char byte = charPtr[i];
		bufferData.push_back(byte);
	}
}

void UniformBuffers::addDataMatrix4x4(SmoothieMath::Matrix4x4* data)
{
	elementDataPointers.push_back(data);
	bufferElementSizes.push_back(sizeof(SmoothieMath::Matrix4x4));

	unsigned char* charPtr = reinterpret_cast<unsigned char*>(data);
	for (unsigned int i = 0; i < sizeof(SmoothieMath::Matrix4x4); i++)
	{
		const unsigned char byte = charPtr[i];
		bufferData.push_back(byte);
	}
}

void UniformBuffers::addDataBool(bool* data)
{
	elementDataPointers.push_back(data);
	bufferElementSizes.push_back(sizeof(int));

	unsigned char* charPtr = reinterpret_cast<unsigned char*>(data);
	for (unsigned int i = 0; i < sizeof(int); i++)
	{
		const unsigned char byte = charPtr[i];
		bufferData.push_back(byte);
	}
}

VkDescriptorSetLayout UniformBuffers::getDescriptorSetLayout() const
{
	return descriptorSetLayout;
}

VkDescriptorSet UniformBuffers::getDescriptorSet() const
{
	return descriptorSet;
}
