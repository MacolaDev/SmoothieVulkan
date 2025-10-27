#include "DeferredModel.h"
#include "Core/SmoothieCore.h"
#include "DeferredPipeline.h"
#include "Effects/Deferred_Core.h"
#include "Core/Image.h"
#include <cstring>

using namespace Smoothie::DeferredRendering;

static inline void fill_descriptor_with_buffer_data(VkDescriptorSet& descriptorSet, unsigned int binding, VkBuffer buffer)
{
	VkDescriptorBufferInfo _buffer_info{};
	_buffer_info.buffer = buffer;
	_buffer_info.offset = 0;
	_buffer_info.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet __write_descriptor_set{};
	__write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	__write_descriptor_set.dstSet = descriptorSet;
	__write_descriptor_set.dstBinding = binding;
	__write_descriptor_set.dstArrayElement = 0;
	__write_descriptor_set.descriptorCount = 1;
	__write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	__write_descriptor_set.pImageInfo = nullptr;
	__write_descriptor_set.pBufferInfo = &_buffer_info;

	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &__write_descriptor_set, 0, nullptr);
}

static std::mutex _add_pipelines_mutex;
int Smoothie::DeferredRendering::StandardModel::create()
{
	auto* __drawingClass = dynamic_cast<Smoothie::DeferredRendering::Drawing*>(SmoothieCore::getDrawerClass());
	if (__drawingClass == nullptr)
	{
		std::cout << "Failed to get drawing class!" << std::endl;
		return 1;
	}


	int __error = 0;
	const ShaderFile& __shader = ShaderFile::getResource(modelFile.getShaderFile(), __error);
	if (__error != 0)
	{
		std::cout << "Failed to create shader!" << std::endl;
		return 1;
	}

	const auto& _pipelines_gbuffer = __drawingClass->getPipelines_gBuffer();
	const auto& _pipelines_hdr = __drawingClass->getPipelines_HDR();
	const auto& __shader_f = modelFile.getShaderFile();


	_add_pipelines_mutex.lock();
	if (_pipelines_gbuffer.find(__shader_f) != _pipelines_gbuffer.end())
	{
		pipeline = _pipelines_gbuffer.at(__shader_f);
	}
	else if (_pipelines_hdr.find(__shader_f) != _pipelines_hdr.end())
	{
		pipeline = _pipelines_hdr.at(__shader_f);
	}
	else
	{
		int _size = __shader.shaders.at({ "STANDARD", VK_SHADER_STAGE_FRAGMENT_BIT }).pipe_outputs.size();

		pipeline = std::make_shared<StaticPipeline>();
		if (pipeline->create(__shader) != 0)
		{
			std::cout << "Failed to create pipeline from shader file!" << std::endl;
			_add_pipelines_mutex.unlock();
			return 1;
		}
		if (_size == 1)
		{
			__drawingClass->addPipeline_HDR(__shader_f, std::dynamic_pointer_cast<StaticPipeline, Pipeline_Base>(pipeline));
		}
		else
		{
			__drawingClass->addPipeline_gBuffer(__shader_f, std::dynamic_pointer_cast<StaticPipeline, Pipeline_Base>(pipeline));
		}
	}
	_add_pipelines_mutex.unlock();

	const StaticPipeline* __pipeline = dynamic_cast<const StaticPipeline*>(pipeline.get());
	if (__pipeline == nullptr)
	{
		std::cout << "Pipeline must be StaticPipeline!" << std::endl;
		return 1;
	}


	//***************************** Descriptors ************************************//
	const VkDescriptorSetLayout __pipeline_descriptor_set_layout = __pipeline->getPipelineDescriptorSetLayout();
	if (__pipeline_descriptor_set_layout == nullptr)
	{
		std::cout << "Draw class descriptor set is not valid!" << std::endl;
		return 1;
	}

	const auto& __descriptor_data = __pipeline->getPipelineDescriptorsData();
	VkDescriptorPoolCreateInfo __descriptorPoolCreateInfo = {};
	__descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	__descriptorPoolCreateInfo.maxSets = 1;
	__descriptorPoolCreateInfo.pNext = nullptr;
	__descriptorPoolCreateInfo.poolSizeCount = __descriptor_data.getPoolSizes().size();
	__descriptorPoolCreateInfo.pPoolSizes = __descriptor_data.getPoolSizes().data();
	if (vkCreateDescriptorPool(SmoothieCore::getDevice(), &__descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor pool!" << std::endl;
		return 1;
	}

	VkDescriptorSetAllocateInfo __allocate_info{};
	__allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	__allocate_info.descriptorPool = descriptorPool;
	__allocate_info.descriptorSetCount = 1;
	__allocate_info.pSetLayouts = &__pipeline_descriptor_set_layout;
	if (vkAllocateDescriptorSets(SmoothieCore::getDevice(), &__allocate_info, &descriptorSet) != VK_SUCCESS)
	{
		std::cout << "Failed to allocate descriptor set!" << std::endl;
		return 1;
	}

	//***************************** Uniform buffer creation ************************************//
	const auto& __pipeline_buffers = __descriptor_data.getBuffers();
	if (__pipeline_buffers.find("ModelData") == __pipeline_buffers.end())
	{
		std::cout << "Pipeline must have a buffer at binding 0!" << std::endl;
		return 1;
	}
	const auto& _uniform_buffer = __pipeline_buffers.at("ModelData");
	if (_uniform_buffer.type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
	{
		std::cout << "Pipeline must have a uniform buffer at binding 0!" << std::endl;
		return 1;
	}

	VkBufferCreateInfo __uniformBufferCreateInfo = {};
	__uniformBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	__uniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	__uniformBufferCreateInfo.size = _uniform_buffer.size;

	VmaAllocationCreateInfo __uniformBufferAllocationCreateInfo = {};
	__uniformBufferAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	__uniformBufferAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &__uniformBufferCreateInfo, &__uniformBufferAllocationCreateInfo, &m_ModelUniformBuffer, &m_ModelUniformBuffferAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create unifrom buffer!" << std::endl;
		return 1;
	}

	const auto& __uniform_buffer_variables = _uniform_buffer.variables;
	std::vector<char> __uniform_buffer_data(_uniform_buffer.size);
	for (const auto& [__name, __type] : __uniform_buffer_variables)
	{
		switch (__type.type)
		{

		default:
			break;
		}
	}

	if (__uniform_buffer_variables.find("modelMatrix") != __uniform_buffer_variables.end())
	{
		const auto& __matrix = __uniform_buffer_variables.at("modelMatrix");
		if (__matrix.type == MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_MAT4)
		{
			std::memcpy(&__uniform_buffer_data[__matrix.offset], &modelMatrix, sizeof(modelMatrix));
		}
	}

	if (vmaCopyMemoryToAllocation(SmoothieCore::getVulkanMemoryAllocator(), __uniform_buffer_data.data(), m_ModelUniformBuffferAllocation, 0, __uniform_buffer_data.size()) != VK_SUCCESS)
	{
		std::cout << "Failed to copy memory to allocation!" << std::endl;
		return 1;
	}


	VkDescriptorBufferInfo __bufferInfo = {};
	__bufferInfo.buffer = m_ModelUniformBuffer;
	__bufferInfo.offset = 0;
	__bufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet __write_descriptor_set{};
	__write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	__write_descriptor_set.dstSet = descriptorSet;
	__write_descriptor_set.dstBinding = _uniform_buffer.binding;
	__write_descriptor_set.dstArrayElement = 0;
	__write_descriptor_set.descriptorCount = 1;
	__write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	__write_descriptor_set.pBufferInfo = &__bufferInfo;
	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &__write_descriptor_set, 0, nullptr);

	//***************************** Textures ************************************//
	const auto& __pipeline_samplers = __descriptor_data.getSamplers();
	for (const auto& [__name, __sampler] : __pipeline_samplers) //Firstly update all textures
	{
		VkDescriptorImageInfo __imageInfo{};
		__imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		__imageInfo.imageView = SmoothieCore::getDefault2DTexture().getImageView();
		__imageInfo.sampler = __drawingClass->getSampler("Texture2DModelSampler");
		VkWriteDescriptorSet __write_descriptor_set{};
		__write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		__write_descriptor_set.dstSet = descriptorSet;
		__write_descriptor_set.dstBinding = __sampler.binding;
		__write_descriptor_set.dstArrayElement = 0;
		__write_descriptor_set.descriptorCount = 1;
		__write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		__write_descriptor_set.pImageInfo = &__imageInfo;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &__write_descriptor_set, 0, nullptr);
	}


	for (const auto& __texture: modelFile.getMaterialProperties())
	{
		if (__texture.name != "Texture") continue;
		
		if (__pipeline_samplers.find(__texture.textContent) != __pipeline_samplers.end())
		{
			int __error = 0;
			const auto& __texture_path = __texture.getChild("value").textContent;
			const auto& __loaded_texture = Smoothie::Texture2D::getResource(__texture_path, __error);
			if (__error != 0) 
			{
				std::cout << "Failed to load: " << __texture.textContent << std::endl;
				continue;
			}
			m_Textures.push_back(__texture_path);

			const auto& __sampler = __pipeline_samplers.at(__texture.textContent);
			VkDescriptorImageInfo __imageInfo{};
			__imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			__imageInfo.imageView = __loaded_texture.getImageView();
			__imageInfo.sampler = __drawingClass->getSampler("Texture2DModelSampler");

			VkWriteDescriptorSet __write_descriptor_set{};
			__write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			__write_descriptor_set.dstSet = descriptorSet;
			__write_descriptor_set.dstBinding = __sampler.binding;
			__write_descriptor_set.dstArrayElement = 0;
			__write_descriptor_set.descriptorCount = 1;
			__write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			__write_descriptor_set.pImageInfo = &__imageInfo;
			vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &__write_descriptor_set, 0, nullptr);
		}
	}

	__error = 0;
	const auto& __mesh = Smoothie::Mesh::getResource(modelFile.getGeometryFile(), __error);
	if (__error != 0)
	{
		std::cout << "Failed to load mesh file: " << modelFile.getGeometryFile() << std::endl;
		return 1;
	}
	m_vertexBuffer = __mesh.vertexBuffer;
	m_indexBuffer = __mesh.indexBuffer;
	m_indexType = __mesh.indexType;
	m_numberOfIndecies = __mesh.numberOfIndices;

	setID(SmoothieCore::generate_random_key());
    return 0;
}

void Smoothie::DeferredRendering::StandardModel::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet drawingClassDescriptor, unsigned int FrameID) const
{
	const VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, m_indexType);

	SmoothieCore::setViewport(commandBuffer);
	SmoothieCore::setScissor(commandBuffer);

	if (doubleSided == 0)
	{
		vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_FRONT_BIT);
	}
	else
	{
		vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
	}
	
	const VkDescriptorSet descriptorSets[] =
	{
		SmoothieCore::getCameraDescriptorSet(),
		drawingClassDescriptor,
		descriptorSet
	};
	
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 3, descriptorSets, 0, nullptr);
	vkCmdDrawIndexed(commandBuffer, m_numberOfIndecies, 1, 0, 0, 0);
}

void Smoothie::DeferredRendering::StandardModel::drawShadow(const SmoothieMath::Matrix4x4 lightProjViewMat, VkPipelineLayout pipelineLayout, VkCommandBuffer commandBuffer, unsigned int FrameID) const
{
}

void Smoothie::DeferredRendering::StandardModel::destroy()
{
	
	//Pipeline itself should be destroyed by the drawing class

	Smoothie::Mesh::removeResource(modelFile.getGeometryFile());
	for (const auto& __texture: m_Textures)
	{
		Smoothie::Texture2D::removeResource(__texture);
	}
	m_Textures.clear();

	vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), m_ModelUniformBuffer, m_ModelUniformBuffferAllocation);
	m_ModelUniformBuffer = nullptr, m_ModelUniformBuffferAllocation = nullptr;
	
	vkDestroyDescriptorPool(SmoothieCore::getDevice(), descriptorPool, nullptr);
	descriptorPool = nullptr, descriptorSet = nullptr;
	ShaderFile::removeResource(modelFile.getShaderFile());
}
