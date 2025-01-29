#pragma once
#include <vulkan/vulkan.h>
#include "ResourceManager/ParseXML.h"
#include <unordered_map>
#include <vector>
#include "Math/SmoothieMath.h"
#include "Core/Image.h"
#include "Core/VMA.h"

struct ModelMatrices
{
	SmoothieMath::Matrix4x4 modelMatrix;
	SmoothieMath::Matrix4x4 normalMatrix;
};


namespace Smoothie 
{
	class Model;
	class ShaderProperty
	{
	public:
		ShaderProperty(const Element& propertyElement, const std::vector<ModelMatrices>& modelMatrices);
		ShaderProperty() = default;
		void update();
		void destroy();

		VkDescriptorSet getDescriptorSet() const;
		VkDescriptorSetLayout getDescriptorSetLayout() const;

		void getProperty(const std::string& propertyName, SmoothieMath::Vector3* vector);
		void setProperty(const std::string& propertyName, SmoothieMath::Vector3* data);

		void getProperty(const std::string& propertyName, SmoothieMath::Vector4* vector);
		void setProperty(const std::string& propertyName, SmoothieMath::Vector4* data);

	private:
		VkDescriptorSet descriptorSet = nullptr;
		VkDescriptorSetLayout descriptorSetLayout = nullptr;
		VkDescriptorPool descriptorPool = nullptr;

		VkBuffer matricesBuffer = nullptr;
		VmaAllocation matricesBufferAllocation = nullptr;

		std::unordered_map<std::string, unsigned int> indexMap;
		std::vector<unsigned char> customPropertyData;
		std::vector<Texture2D> textures;
	};

}