#include "ShaderProperty.h"

#define _SMOOTHIE_ENGINE
#include "Core/SmoothieCore.h"
using namespace Smoothie;
using namespace SmoothieMath;

ShaderProperty::ShaderProperty(const Element& propertyElement, const std::vector<ModelMatrices>& modelMatrices)
{
	for (int i = 0; i < propertyElement.children.size(); i++)
	{
		Element shaderProperty = propertyElement.children[i];
		const std::string propertyType = shaderProperty.name;
		const std::string propertyName = shaderProperty.textContent;
		const std::string propertyValue = shaderProperty.children[0].textContent;
	
		if (propertyType == "Float")
		{
			const float value = shaderProperty.children[0].getFloat();
		}

		if (propertyType == "Vector3")
		{
			const Vector3 value = shaderProperty.children[0].getVector3();
		}

		if (propertyType == "Vector4")
		{
			const Vector4 value = shaderProperty.children[0].getVector4();
		}


		if (propertyType == "Texture")
		{
			textures.push_back(Texture2D(shaderProperty.children[0].textContent));
		}
	}

	//model mitrices pool
	VkDescriptorPoolSize modelMatricesPool{};
	modelMatricesPool.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	modelMatricesPool.descriptorCount = 1;

	//Texture descriptor pool
	VkDescriptorPoolSize texturePoolSize{};
	texturePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	texturePoolSize.descriptorCount = static_cast<unsigned int>(textures.size());

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	
	VkDescriptorPoolSize poolSizes[] = { texturePoolSize, modelMatricesPool };
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = poolSizes;
	
	poolInfo.maxSets = 1;
	vkCreateDescriptorPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &descriptorPool);

	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayouts;
	
	//Matrices layouts
	VkDescriptorSetLayoutBinding matricesLayout{};
	matricesLayout.binding = 0;
	matricesLayout.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	matricesLayout.descriptorCount = 3;
	matricesLayout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	matricesLayout.pImmutableSamplers = nullptr;
	descriptorSetLayouts.push_back(matricesLayout);


	//Texture layouts
	for (int i = 0; i < textures.size(); i++)
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = i + 1;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBinding.pImmutableSamplers = nullptr;
		descriptorSetLayouts.push_back(layoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<unsigned int>(descriptorSetLayouts.size());
	layoutInfo.pBindings = descriptorSetLayouts.data();
	vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &layoutInfo, nullptr, &descriptorSetLayout);


	//Creating descriptor set
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
	vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &descriptorSet);
	
	//Create matrices buffer
	VkBufferCreateInfo modelMatricesBufferInfo{};
	modelMatricesBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	modelMatricesBufferInfo.size = sizeof(ModelMatrices) * modelMatrices.size();
	modelMatricesBufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	VmaAllocationCreateInfo vmaModelMatricesBufferInfo{};
	vmaModelMatricesBufferInfo.usage = VMA_MEMORY_USAGE_AUTO;
	vmaModelMatricesBufferInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	vmaCreateBuffer(VMA::getAllocator(), &modelMatricesBufferInfo, &vmaModelMatricesBufferInfo, &matricesBuffer, &matricesBufferAllocation, nullptr);
	vmaCopyMemoryToAllocation(VMA::getAllocator(), modelMatrices.data(), matricesBufferAllocation, 0, modelMatricesBufferInfo.size);
	
	//Updating descriptor set with model matrices
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = matricesBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr;
	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);



	//Updating descriptor sets with textures
	for (int i = 0; i < textures.size(); i++) 
	{
		VkDescriptorImageInfo imgInfo{};
		imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgInfo.imageView = textures[i].image.imageView;
		imgInfo.sampler = Samplers::Texture2DModelSampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.dstBinding = i + 1;
		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imgInfo;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
	}
}

void ShaderProperty::update()
{
}

void ShaderProperty::destroy()
{
	vmaDestroyBuffer(VMA::getAllocator(), matricesBuffer, matricesBufferAllocation);
	matricesBuffer = nullptr;
	matricesBufferAllocation = nullptr;
	for (int i = 0; i < textures.size(); i++) 
	{
		textures[i].destroy();
	}
}

VkDescriptorSet ShaderProperty::getDescriptorSet() const
{
	return descriptorSet;
}

VkDescriptorSetLayout ShaderProperty::getDescriptorSetLayout() const
{
	return descriptorSetLayout;
}
