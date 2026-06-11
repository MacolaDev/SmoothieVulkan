#include "Standard.h"
#include "Core/SmoothieCore.h"
#include "../Pipelines/Standard.h"

#include <cassert>

#include "Effects/Deferred_Core.h"
#include <cstring>
#include <iostream>

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


int Smoothie::DeferredRendering::Model_Standard::create()
{
    assert(m_Filepath.size() > 0);

	auto _drawingClass = std::dynamic_pointer_cast<Smoothie::DeferredRendering::Drawing>(SmoothieCore::getDrawingClassPtr());
	if (_drawingClass == nullptr)
	{
		std::cout << "Failed to get drawing class!" << std::endl;
	    destroy();
		return 1;
	}



	int _error = 0;
	// const ShaderFile& _shader = ShaderFile::getResource(m_modelFile.getShaderFile(), _error);
	// if (_error != 0)
	// {
	// 	std::cout << "Failed to create shader!" << std::endl;
	// 	return 1;
	// }

	// const auto& _pipelines_gbuffer = _drawingClass->getPipelines_gBuffer();
	// const auto& _pipelines_hdr = _drawingClass->getPipelines_HDR();
	// const auto& _shader_f = m_modelFile.getShaderFile();


	// _add_pipelines_mutex.lock();
	// if (_pipelines_gbuffer.find(_shader_f) != _pipelines_gbuffer.end())
	// {
	// 	m_pPipeline = _pipelines_gbuffer.at(_shader_f);
	// }
	// else if (_pipelines_hdr.find(_shader_f) != _pipelines_hdr.end())
	// {
	// 	m_pPipeline = _pipelines_hdr.at(_shader_f);
	// }
	// else
	// {
	// 	int _size = _shader.shaders.at({ "STANDARD", VK_SHADER_STAGE_FRAGMENT_BIT }).pipe_outputs.size();
	//
	// 	m_pPipeline = std::make_shared<StaticPipeline>();
	// 	m_pPipeline->setShaderFile(_shader);
	// 	if (m_pPipeline->create() != 0)
	// 	{
	// 		std::cout << "Failed to create pipeline from shader file!" << std::endl;
	// 		_add_pipelines_mutex.unlock();
	// 		return 1;
	// 	}
	// 	if (_size == 1)
	// 	{
	// 		_drawingClass->addPipeline_HDR(_shader_f, std::dynamic_pointer_cast<StaticPipeline, Pipeline_Base>(m_pPipeline));
	// 	}
	// 	else
	// 	{
	// 		_drawingClass->addPipeline_gBuffer(_shader_f, std::dynamic_pointer_cast<StaticPipeline, Pipeline_Base>(m_pPipeline));
	// 	}
	// }
	// _add_pipelines_mutex.unlock();

	// const auto* _pipeline = dynamic_cast<const StaticPipeline*>(m_pPipeline.get());
	// if (_pipeline == nullptr)
	// {
	// 	std::cout << "Pipeline must be StaticPipeline!" << std::endl;
	// 	return 1;
	// }


	//***************************** Descriptors ************************************//
	// VkDescriptorSetLayout _pipeline_descriptor_set_layout = _pipeline->getPipelineDescriptorSetLayout();
	// if (_pipeline_descriptor_set_layout == nullptr)
	// {
	// 	std::cout << "Draw class descriptor set is not valid!" << std::endl;
	// 	return 1;
	// }

	//const auto& _descriptor_data = _pipeline->getDescriptorData();
	VkDescriptorPoolCreateInfo _descriptorPoolCreateInfo = {};
	_descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	_descriptorPoolCreateInfo.maxSets = 1;
	_descriptorPoolCreateInfo.pNext = nullptr;
	//_descriptorPoolCreateInfo.poolSizeCount = _descriptor_data.getPoolSizes().size();
	//_descriptorPoolCreateInfo.pPoolSizes = _descriptor_data.getPoolSizes().data();
	if (vkCreateDescriptorPool(SmoothieCore::getDevice(), &_descriptorPoolCreateInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor pool!" << std::endl;
		return 1;
	}

	VkDescriptorSetAllocateInfo _allocate_info{};
	_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	_allocate_info.descriptorPool = m_DescriptorPool;
	_allocate_info.descriptorSetCount = 1;
	//_allocate_info.pSetLayouts = &_pipeline_descriptor_set_layout;
	if (vkAllocateDescriptorSets(SmoothieCore::getDevice(), &_allocate_info, &m_DescriptorSet) != VK_SUCCESS)
	{
		std::cout << "Failed to allocate descriptor set!" << std::endl;
		return 1;
	}

	//***************************** Uniform buffer creation ************************************//
	// const auto& _pipeline_buffers = _descriptor_data.getBuffers();
	// if (_pipeline_buffers.find("ModelData") == _pipeline_buffers.end())
	// {
	// 	std::cout << "Pipeline must have a buffer at binding 0!" << std::endl;
	// 	return 1;
	// }
	// const auto& _uniform_buffer = _pipeline_buffers.at("ModelData");
	// if (_uniform_buffer.type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
	// {
	// 	std::cout << "Pipeline must have a uniform buffer at binding 0!" << std::endl;
	// 	return 1;
	// }

	// VkBufferCreateInfo _uniformBufferCreateInfo = {};
	// _uniformBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	// _uniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	// //_uniformBufferCreateInfo.size = _uniform_buffer.size;
	//
	// VmaAllocationCreateInfo _uniformBufferAllocationCreateInfo = {};
	// _uniformBufferAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	// _uniformBufferAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	// if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &_uniformBufferCreateInfo, &_uniformBufferAllocationCreateInfo, &m_ModelUniformBuffer, &m_ModelUniformBufferAllocation, nullptr) != VK_SUCCESS)
	// {
	// 	std::cout << "Failed to create unifrom buffer!" << std::endl;
	// 	return 1;
	// }

	//const auto& _uniform_buffer_variables = _uniform_buffer.variables;
	//std::vector<char> _uniform_buffer_data(_uniform_buffer.size);
	// for (const auto& [_name, _type] : _uniform_buffer_variables)
	// {
	// 	switch (_type.type)
	// 	{
	//
	// 	default:
	// 		break;
	// 	}
	// }
	//
	// if (_uniform_buffer_variables.find("modelMatrix") != _uniform_buffer_variables.end())
	// {
	// 	const auto& _matrix = _uniform_buffer_variables.at("modelMatrix");
	// 	if (_matrix.type == MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_MAT4)
	// 	{
	// 		std::memcpy(&_uniform_buffer_data[_matrix.offset], &modelMatrix, sizeof(modelMatrix));
	// 	}
	// }
	//
	// if (vmaCopyMemoryToAllocation(SmoothieCore::getVulkanMemoryAllocator(), _uniform_buffer_data.data(), m_ModelUniformBufferAllocation, 0, _uniform_buffer_data.size()) != VK_SUCCESS)
	// {
	// 	std::cout << "Failed to copy memory to allocation!" << std::endl;
	// 	return 1;
	// }
	//
	//
	// VkDescriptorBufferInfo _bufferInfo = {};
	// _bufferInfo.buffer = m_ModelUniformBuffer;
	// _bufferInfo.offset = 0;
	// _bufferInfo.range = VK_WHOLE_SIZE;
	//
	// VkWriteDescriptorSet _write_descriptor_set{};
	// _write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	// _write_descriptor_set.dstSet = descriptorSet;
	// _write_descriptor_set.dstBinding = _uniform_buffer.binding;
	// _write_descriptor_set.dstArrayElement = 0;
	// _write_descriptor_set.descriptorCount = 1;
	// _write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	// _write_descriptor_set.pBufferInfo = &_bufferInfo;
	// vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_write_descriptor_set, 0, nullptr);
	//
	// //***************************** Textures ************************************//
	// const auto& _pipeline_samplers = _descriptor_data.getSamplers();
	// for (const auto& [_name, _sampler] : _pipeline_samplers) //Firstly update all textures
	// {
	// 	VkDescriptorImageInfo _imageInfo{};
	// 	_imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	// 	_imageInfo.imageView = SmoothieCore::getDefault2DTexture().getImageView();
	// 	_imageInfo.sampler = _drawingClass->getSampler("Texture2DModelSampler");
	// 	VkWriteDescriptorSet _write_descriptor_set{};
	// 	_write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	// 	_write_descriptor_set.dstSet = descriptorSet;
	// 	_write_descriptor_set.dstBinding = _sampler.binding;
	// 	_write_descriptor_set.dstArrayElement = 0;
	// 	_write_descriptor_set.descriptorCount = 1;
	// 	_write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	// 	_write_descriptor_set.pImageInfo = &_imageInfo;
	// 	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_write_descriptor_set, 0, nullptr);
	// }
	//
	//
	// for (const auto& _texture: m_modelFile.getMaterialProperties())
	// {
	// 	if (_texture.name != "Texture") continue;
	//
	// 	if (_pipeline_samplers.find(_texture.textContent) != _pipeline_samplers.end())
	// 	{
	// 		int _error = 0;
	// 		const auto& __texture_path = _texture.getChild("value").textContent;
	// 		const auto& __loaded_texture = Smoothie::Texture2D::getResource(__texture_path, _error);
	// 		if (_error != 0)
	// 		{
	// 			std::cout << "Failed to load: " << _texture.textContent << std::endl;
	// 			continue;
	// 		}
	// 		m_Textures.push_back(__texture_path);
	//
	// 		const auto& __sampler = _pipeline_samplers.at(_texture.textContent);
	// 		VkDescriptorImageInfo _imageInfo{};
	// 		_imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	// 		_imageInfo.imageView = __loaded_texture.getImageView();
	// 		_imageInfo.sampler = _drawingClass->getSampler("Texture2DModelSampler");
	//
	// 		VkWriteDescriptorSet _write_descriptor_set{};
	// 		_write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	// 		_write_descriptor_set.dstSet = descriptorSet;
	// 		_write_descriptor_set.dstBinding = __sampler.binding;
	// 		_write_descriptor_set.dstArrayElement = 0;
	// 		_write_descriptor_set.descriptorCount = 1;
	// 		_write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	// 		_write_descriptor_set.pImageInfo = &_imageInfo;
	// 		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_write_descriptor_set, 0, nullptr);
	// 	}
	// }
	//
	// _error = 0;
	// const auto& _mesh = Smoothie::Mesh::getResource(m_modelFile.getGeometryFile(), _error);
	// if (_error != 0)
	// {
	// 	std::cout << "Failed to load mesh file: " << m_modelFile.getGeometryFile() << std::endl;
	// 	return 1;
	// }
	// m_vertexBuffer = _mesh.vertexBuffer;
	// m_indexBuffer = _mesh.indexBuffer;
	// m_indexType = _mesh.indexType;
	// m_numberOfIndices = _mesh.numberOfIndices;
	//
	//
	// _pipeline->add_to_rendering(std::make_shared<Standard_Model>(*this));
    return 0;
}

int Smoothie::DeferredRendering::Model_Standard::update()
{
	return 0;
}

void Smoothie::DeferredRendering::Model_Standard::draw(
                VkCommandBuffer commandBuffer,
                VkPipelineLayout pipelineLayout,
                VkDescriptorSet drawingClassDescriptor,
                unsigned int ImageIndex) const
{
	const VkDeviceSize _offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer, _offsets);
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
		//SmoothieCore::getCameraDescriptorSet(),
		drawingClassDescriptor,
		m_DescriptorSet
	};
	
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 3, descriptorSets, 0, nullptr);
	vkCmdDrawIndexed(commandBuffer, m_numberOfIndices, 1, 0, 0, 0);
}

// void Smoothie::DeferredRendering::Standard_Model::drawShadow(const SmoothieMath::Matrix4x4 lightProjViewMat, VkPipelineLayout pipelineLayout, VkCommandBuffer commandBuffer, unsigned int FrameID) const
// {
// }

void Smoothie::DeferredRendering::Model_Standard::add_model_matrix(const SmoothieMath::Matrix4x4& matrix)
{

}

void Smoothie::DeferredRendering::Model_Standard::destroy()
{

	//Pipeline itself should be destroyed by the drawing class

    if (m_ModelUniformBuffer != nullptr && m_ModelUniformBufferAllocation != nullptr)
    {
        vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), m_ModelUniformBuffer, m_ModelUniformBufferAllocation);
        m_ModelUniformBuffer = nullptr, m_ModelUniformBufferAllocation = nullptr;
    }

    if (m_DescriptorPool != nullptr)
    {
        vkDestroyDescriptorPool(SmoothieCore::getDevice(), m_DescriptorPool, nullptr);
        m_DescriptorPool = nullptr, m_DescriptorSet = nullptr;
    }

    m_Filepath.erase();
}
