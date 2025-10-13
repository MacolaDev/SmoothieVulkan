#pragma once
#include <vulkan/vulkan.h>

#include <vector>
#include <unordered_set>

#include "Core/Buffer.h"
#include "Core/Texture.h"
#include "Core/Shader.h"

namespace Smoothie
{
	//***********************************
	//Base class for all descriptor sets.
	//***********************************
	class DescriptorBase
	{
	public:
		virtual int create() = 0;
		virtual void resize_callback() = 0;
		virtual void destroy() = 0;

		inline VkDescriptorSet getDescriptrotSet() const { return descriptorSet; }
		inline VkDescriptorSetLayout getDescriptrotSetLayout() const { return descriptorSetLayout; }
		inline VkDescriptorPool getDescriptrotPool() const { return descriptorPool; }
		DescriptorBase() = default;

	protected:
		VkDescriptorSet descriptorSet = nullptr;
		VkDescriptorSetLayout descriptorSetLayout = nullptr;
		VkDescriptorPool descriptorPool = nullptr;
	};


	struct DescriptorBufferVariable
	{
		int size = 0; //Size of this object in bytes. If its an array, this is the size of the entire array in bytes.
		int offset = 0; //Offset of this variable from the start of the buffer.
		ModelDescriptorDataType type = MODEL_DESCRIPTOR_DATA_TYPE_NO_TYPE;
	};

	struct DescriptorBuffer
	{
		int binding = -1;
		VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM; //Either uniform or storage buffer
		unsigned int size = 0; //Size of the buffer in bytes.
		std::unordered_map<std::string, DescriptorBufferVariable> variables; //Variables that this buffer contains. Note that multiple DescriptorBufferVariable may point to the same memory location.
	};

	struct DescriptorSampler 
	{
		int binding = -1;
		ModelDescriptorDataType type = MODEL_DESCRIPTOR_DATA_TYPE_NO_TYPE; //It can be sampler or combined image sampler.
	};


	//This class is used to simplify the mess that is caused by having multiple sets with multiple bindings in multiple shaders
	//that can all be potentailly conflicting with one another.
	//To make creating descriptors easier, this class allow us to provide a shader file, target descriptor set index and shaders that this set will be in use inside of it. 
	//It returns data needed to create that descriptor set. Stick with using regular combined image samplers, uniform and storage buffers in shaders and nobody will be getting hurt :D 
	class DescriptorSetData
	{
		std::vector<VkDescriptorPoolSize> m_PoolSizes;
		std::vector<VkDescriptorSetLayoutBinding> m_LayoutBindings;

		//Reflected buffers. 
		//Key is its binding inside the descriptor set.
		std::unordered_map<std::string, DescriptorBuffer> m_Buffers;

		//Reflected samplers.
		//Key is its binding inside the descriptor set.
		std::unordered_map<std::string, DescriptorSampler> m_Samplers;

	public:

		int create(const ShaderFile& shaderFile, unsigned int targetSetIndex, const std::vector<ShaderFile::SearchKey>& searchKeys);
		inline const std::vector<VkDescriptorPoolSize>& getPoolSizes() const { return m_PoolSizes; }
		inline const std::vector<VkDescriptorSetLayoutBinding>& getDescriptorBindings() const { return m_LayoutBindings; }
		inline const decltype(m_Buffers)& getBuffers() const { return m_Buffers; }
		inline const decltype(m_Samplers)& getSamplers() const { return m_Samplers; }

	};

}