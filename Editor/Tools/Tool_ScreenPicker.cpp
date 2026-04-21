//
// Created by macola on 11/18/25.
//

#include "../EditorCore.h"
#include "Tool_ScreenPicker.h"
#include "Smoothie.h"
#include <algorithm>
#include <iostream>

using namespace SmoothieEditor;

int Tool_ScreenPicker::create()
{
	m_pCore = std::dynamic_pointer_cast<SmoothieEditor::Core>(SmoothieCore::getDrawingClassPtr()->get_editor_corePtr());
	assert(m_pCore != nullptr);

	std::shared_ptr<Smoothie::DeferredRendering::Drawing> _draw_class = std::dynamic_pointer_cast<Smoothie::DeferredRendering::Drawing>(SmoothieCore::getDrawingClassPtr());
	if (_draw_class == nullptr)
	{
		std::cout << "Draw class must be the child of DeferredRendering::Drawing class!" << std::endl;
		assert(_draw_class != nullptr);
		return 1;
	}


    VkImageCreateInfo _imageCreateInfo = {};
    _imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    _imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    _imageCreateInfo.format = VK_FORMAT_R32_UINT;
    _imageCreateInfo.extent.width = SmoothieCore::getScrWidth();
    _imageCreateInfo.extent.height = SmoothieCore::getScrHeight();
    _imageCreateInfo.extent.depth = 1;
    _imageCreateInfo.mipLevels = 1;
    _imageCreateInfo.arrayLayers = 1;
    _imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    _imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    _imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    _imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    _imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo _allocationCreateInfo = {};
    _allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    VkImageViewCreateInfo _imageViewCreateInfo = {};
    _imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    _imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    _imageViewCreateInfo.format = _imageCreateInfo.format;
    _imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    _imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    _imageViewCreateInfo.subresourceRange.layerCount = 1;
    _imageViewCreateInfo.subresourceRange.levelCount = 1;
    _imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    _imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    _imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    _imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;

    if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_imageCreateInfo, &_allocationCreateInfo, &m_ModelIDsImage, &m_ModelIDsImageAllocation, nullptr) != VK_SUCCESS)
    {
        std::cout << "Failed to create Model ids image!" << std::endl;
        return 1;
    }

    if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_imageCreateInfo, &_allocationCreateInfo, &m_InstanceIDsImage, &m_InstanceIDsImageAllocation, nullptr) != VK_SUCCESS)
    {
        std::cout << "Failed to create instance ids image!" << std::endl;
        return 1;
    }


    _imageViewCreateInfo.image = m_ModelIDsImage;
    if (vkCreateImageView(SmoothieCore::getDevice(), &_imageViewCreateInfo, nullptr, &m_ModelIDsImageView) != VK_SUCCESS)
    {
        std::cout << "Failed to create Model ids view!" << std::endl;
        return 1;
    }

    _imageViewCreateInfo.image = m_InstanceIDsImage;
    if (vkCreateImageView(SmoothieCore::getDevice(), &_imageViewCreateInfo, nullptr, &m_InstanceIDsImageView) != VK_SUCCESS)
    {
        std::cout << "Failed to create instance ids view!" << std::endl;
        return 1;
    }

    _imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    _imageCreateInfo.format = VK_FORMAT_D32_SFLOAT;
    if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_imageCreateInfo, &_allocationCreateInfo, &m_pickDepthImage, &m_pickDepthImageAllocation, nullptr) != VK_SUCCESS)
    {
        std::cout << "Failed to create Model ids image!" << std::endl;
        return 1;
    }

    _imageViewCreateInfo.image = m_pickDepthImage;
    _imageViewCreateInfo.format = _imageCreateInfo.format;
    _imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (vkCreateImageView(SmoothieCore::getDevice(), &_imageViewCreateInfo, nullptr, &m_pickDepthImageView) != VK_SUCCESS)
    {
        std::cout << "Failed to create instance ids view!" << std::endl;
        return 1;
    }


    //************************* Descriptor set layout ***************************//
    VkDescriptorSetLayoutBinding _resultBufferSetBinding = {};
    _resultBufferSetBinding.binding = 0;
    _resultBufferSetBinding.descriptorCount = 1;
    _resultBufferSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    _resultBufferSetBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding _depthImageSetBinding = {};
    _depthImageSetBinding.binding = 1;
    _depthImageSetBinding.descriptorCount = 1;
    _depthImageSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    _depthImageSetBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding _modelIDsImageSetBinding = _depthImageSetBinding;
    _modelIDsImageSetBinding.binding = 2;
	_modelIDsImageSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    VkDescriptorSetLayoutBinding _instanceIDsImageSetBinding = _modelIDsImageSetBinding;
    _instanceIDsImageSetBinding.binding = 3;
	_instanceIDsImageSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    const VkDescriptorSetLayoutBinding _descriptorSetBindings[] = { _resultBufferSetBinding, _depthImageSetBinding, _modelIDsImageSetBinding, _instanceIDsImageSetBinding };
    VkDescriptorSetLayoutCreateInfo _setLayoutCreateInfo = {};
    _setLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    _setLayoutCreateInfo.bindingCount = 4;
    _setLayoutCreateInfo.pBindings = _descriptorSetBindings;
    if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &_setLayoutCreateInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
    {
        std::cout << "Failed to create descriptor set layout!" << std::endl;
        return 1;
    }


	m_buffers.resize(SMOOTHIE_MAX_FRAMES_IN_FLIGHT);
	VkSampler _sampler = _draw_class->getSampler("Texture2DModelSampler");
	if (_sampler == nullptr)
	{
		std::cout << "Failed to get the sampler!" << std::endl;
		assert(_sampler != nullptr);
		return 1;
	}

    for (auto& _buffer: m_buffers)
	{
		//************************* Create descriptor pool ***************************//
    	VkDescriptorPoolSize _bufferPoolSize = {};
    	_bufferPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    	_bufferPoolSize.descriptorCount = 1;

    	VkDescriptorPoolSize _depthImagePoolSizes = {};
		_depthImagePoolSizes.descriptorCount = 1;
		_depthImagePoolSizes.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    	VkDescriptorPoolSize _imagePoolSizes = {};
    	_imagePoolSizes.descriptorCount = 2;
    	_imagePoolSizes.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

		const VkDescriptorPoolSize _poolSizes[] = { _bufferPoolSize, _depthImagePoolSizes, _imagePoolSizes };
		VkDescriptorPoolCreateInfo _poolCreateInfo = {};
		_poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		_poolCreateInfo.maxSets = 1;
		_poolCreateInfo.poolSizeCount = 3;
		_poolCreateInfo.pPoolSizes = _poolSizes;
		if (vkCreateDescriptorPool(SmoothieCore::getDevice(), &_poolCreateInfo, nullptr, &_buffer.descriptorPool) != VK_SUCCESS)
		{
			std::cout << "Failed to create descriptor pool!" << std::endl;
			return 1;
		}

		//************************* Allocate descriptor sets ***************************//
		VkDescriptorSetAllocateInfo _allocInfo = {};
		_allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		_allocInfo.descriptorPool = _buffer.descriptorPool;
		_allocInfo.descriptorSetCount = 1;
		_allocInfo.pSetLayouts = &m_descriptorSetLayout;
		if (vkAllocateDescriptorSets(SmoothieCore::getDevice(), &_allocInfo, &_buffer.descriptorSet) != VK_SUCCESS)
		{
			std::cout << "Failed to allocate descriptor set!" << std::endl;
			return 1;
		}

		//************************* Create result buffer *****************************//

		VkBufferCreateInfo _bufferCreateInfo = {};
		_bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		_bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		_bufferCreateInfo.size = sizeof(ScreenPickData);
		_bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo _bufferAllocationCreateInfo = {};
		_bufferAllocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &_bufferCreateInfo, &_bufferAllocationCreateInfo, &_buffer.buffer, &_buffer.allocation, nullptr) != VK_SUCCESS)
		{
			std::cout << "Failed to create buffer!" << std::endl;
			return 1;
		}

		//************************* Update descriptor sets ***************************//

		VkDescriptorBufferInfo _bufferInfo = {};
		_bufferInfo.buffer = _buffer.buffer;
		_bufferInfo.offset = 0;
		_bufferInfo.range = VK_WHOLE_SIZE;


		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = _buffer.descriptorSet;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &_bufferInfo;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);

		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.pBufferInfo = nullptr;
		VkDescriptorImageInfo imgInfo{};
		imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgInfo.sampler = _sampler;

    	imgInfo.imageView = _draw_class->getHdrPass().gDepth.getImageView();
		descriptorWrite.dstBinding = 1;
		descriptorWrite.pImageInfo = &imgInfo;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);

    	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    	descriptorWrite.pBufferInfo = nullptr;
    	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    	imgInfo.sampler = nullptr;

    	imgInfo.imageView = m_ModelIDsImageView;
    	descriptorWrite.dstBinding = 2;
    	descriptorWrite.pImageInfo = &imgInfo;
    	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);

    	imgInfo.imageView = m_InstanceIDsImageView;
    	descriptorWrite.dstBinding = 3;
    	descriptorWrite.pImageInfo = &imgInfo;
    	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	VkPushConstantRange _push_constant = {};
	_push_constant.offset = 0;
	_push_constant.size = sizeof(ImVec2);
	_push_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkPipelineLayoutCreateInfo _pipelineLayoutCreateInfo = {};
	_pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	_pipelineLayoutCreateInfo.setLayoutCount = 2;
	const VkDescriptorSetLayout _descriptorSetLayouts[] = {SmoothieCore::getCameraDescriptorSetLayout(0), m_descriptorSetLayout};
	_pipelineLayoutCreateInfo.pSetLayouts = _descriptorSetLayouts;
	_pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	_pipelineLayoutCreateInfo.pPushConstantRanges = &_push_constant;
	if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &_pipelineLayoutCreateInfo, nullptr, &m_sampleDataPipelineLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create pipeline layout!" << std::endl;
		assert(m_sampleDataPipelineLayout != nullptr);
		return 1;
	}

	VkPipelineShaderStageCreateInfo _shaderCreateInfo = {};
	_shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	_shaderCreateInfo.pName = "main";
	_shaderCreateInfo.module = m_pCore->get_editor_shader_module("GPU_SELECTED_DATA");
	assert(_shaderCreateInfo.module != nullptr);
	_shaderCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;

	VkComputePipelineCreateInfo _pipelineCreateInfo = {};
	_pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	_pipelineCreateInfo.layout = m_sampleDataPipelineLayout;
	_pipelineCreateInfo.stage = _shaderCreateInfo;
	if (vkCreateComputePipelines(SmoothieCore::getDevice(), nullptr, 1, &_pipelineCreateInfo, nullptr, &m_sampleDataPipline) != VK_SUCCESS)
	{
		std::cout << "Failed to create pipeline!" << std::endl;
		assert(m_sampleDataPipline != nullptr);
		return 1;
	}

    return 0;
}

void Tool_ScreenPicker::on_ui_icon_draw()
{
	const auto& winPos = ImGui::GetWindowPos();
	const auto& winSize = ImGui::GetWindowSize();
	const auto& mousePos = ImGui::GetMousePos();

	ImVec2 mousePosWin;
	mousePosWin.x = mousePos.x - winPos.x;
	mousePosWin.y = mousePos.y - winPos.y;

	m_uv.x = std::clamp((mousePosWin.x / winSize.x), 0.0f, 1.0f);
	m_uv.y = 1.0f - std::clamp((mousePosWin.y / winSize.y), 0.0f, 1.0f);
}

void Tool_ScreenPicker::on_command_recording_compute(VkCommandBuffer commandBuffer)
{
	VkImageMemoryBarrier _base_barrier = {};
	_base_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	_base_barrier.pNext = nullptr;
	_base_barrier.image = nullptr;
	_base_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_base_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	_base_barrier.srcAccessMask = 0;
	_base_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	_base_barrier.subresourceRange.layerCount = 1;
	_base_barrier.subresourceRange.baseMipLevel = 0;
	_base_barrier.subresourceRange.levelCount = 1;
	_base_barrier.subresourceRange.baseMipLevel = 0;
	_base_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageMemoryBarrier _modelIDsImageMemoryBarrier = _base_barrier;
	_modelIDsImageMemoryBarrier.image = m_ModelIDsImage;

	VkImageMemoryBarrier _instanceIDsImageMemoryBarrier = _base_barrier;
	_instanceIDsImageMemoryBarrier.image = m_InstanceIDsImage;

	VkImageMemoryBarrier _depthIDsImageMemoryBarrier = _base_barrier;
	_depthIDsImageMemoryBarrier.image = m_pickDepthImage;
	_depthIDsImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	_depthIDsImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	_depthIDsImageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkImageMemoryBarrier _image_barriers[] = {_modelIDsImageMemoryBarrier, _instanceIDsImageMemoryBarrier, _depthIDsImageMemoryBarrier};

	vkCmdPipelineBarrier(commandBuffer,  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0,
		0, nullptr,
		0, nullptr,
		3, _image_barriers);


    VkRenderingAttachmentInfo _modelIDsRenderingAttachment = {};
    _modelIDsRenderingAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    _modelIDsRenderingAttachment.pNext = nullptr;
    _modelIDsRenderingAttachment.imageView = m_ModelIDsImageView;
    _modelIDsRenderingAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _modelIDsRenderingAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    _modelIDsRenderingAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    _modelIDsRenderingAttachment.clearValue.color.uint32[0] = 1;

    VkRenderingAttachmentInfo _instanceIDsRenderingAttachment = _modelIDsRenderingAttachment;
    _instanceIDsRenderingAttachment.imageView = m_InstanceIDsImageView;

    VkRenderingAttachmentInfo _depthRenderingAttachment = _modelIDsRenderingAttachment;
    _depthRenderingAttachment.imageView = m_pickDepthImageView;
    _depthRenderingAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    _depthRenderingAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    _depthRenderingAttachment.clearValue.depthStencil.depth = 0.0f;

    VkRenderingInfo _renderingInfo = {};
    _renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    _renderingInfo.renderArea.extent.height = SmoothieCore::getScrHeight();
    _renderingInfo.renderArea.extent.width = SmoothieCore::getScrWidth();
    _renderingInfo.renderArea.offset = {0, 0};
    _renderingInfo.layerCount = 1;

    const VkRenderingAttachmentInfo _colorAttachments[] = {_modelIDsRenderingAttachment, _instanceIDsRenderingAttachment};
    _renderingInfo.colorAttachmentCount = 2;
    _renderingInfo.pColorAttachments = _colorAttachments;
    _renderingInfo.pDepthAttachment = &_depthRenderingAttachment;
    vkCmdBeginRendering(commandBuffer, &_renderingInfo);

    if (m_pCore != nullptr)
    {
        auto& _tools = m_pCore->get_tools();
        for (auto& [key, tool] : _tools)
        {
            tool->on_screen_pick_record(commandBuffer);
        }
    }

    vkCmdEndRendering(commandBuffer);

	_base_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	_base_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	_base_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	_base_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	_modelIDsImageMemoryBarrier = _base_barrier;
	_modelIDsImageMemoryBarrier.image = m_ModelIDsImage;

	_instanceIDsImageMemoryBarrier = _base_barrier;
	_instanceIDsImageMemoryBarrier.image = m_InstanceIDsImage;

	_depthIDsImageMemoryBarrier = _base_barrier;
	_depthIDsImageMemoryBarrier.image = m_pickDepthImage;
	_depthIDsImageMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	_depthIDsImageMemoryBarrier.dstAccessMask = 0;
	_depthIDsImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	_depthIDsImageMemoryBarrier.oldLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	_depthIDsImageMemoryBarrier.oldLayout  = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImageMemoryBarrier _image_barriers2[] = {_modelIDsImageMemoryBarrier, _instanceIDsImageMemoryBarrier, _depthIDsImageMemoryBarrier};

	VkMemoryBarrier _compute_barrier = {};
	_compute_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	_compute_barrier.srcAccessMask = VK_ACCESS_HOST_READ_BIT;
	_compute_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
		1, &_compute_barrier,
		0, nullptr,
		3, _image_barriers2);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_sampleDataPipline);
	const VkDescriptorSet _sets[] =
	{
		SmoothieCore::getCameraDescriptorSet(),
		m_buffers[SmoothieCore::getCurrentFrame()].descriptorSet
	};
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_sampleDataPipelineLayout, 0, 2, _sets, 0, nullptr);
	vkCmdPushConstants(commandBuffer, m_sampleDataPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ImVec2), &m_uv);
	vkCmdDispatch(commandBuffer, 1, 1, 1);

	_compute_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	_compute_barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,  0,
		1, &_compute_barrier,
		0, nullptr,
		0, nullptr);

}

void Tool_ScreenPicker::on_command_execution_finish()
{
	const auto& _buffer = m_buffers[SmoothieCore::getCurrentFrame()];
	vmaCopyAllocationToMemory(SmoothieCore::getVulkanMemoryAllocator(), _buffer.allocation, 0, &m_data, sizeof(ScreenPickData));
}

int Tool_ScreenPicker::on_resize()
{

	std::shared_ptr<Smoothie::DeferredRendering::Drawing> _draw_class = std::dynamic_pointer_cast<Smoothie::DeferredRendering::Drawing>(SmoothieCore::getDrawingClassPtr());
	if (_draw_class == nullptr)
	{
		std::cout << "Draw class must be the child of DeferredRendering::Drawing class!" << std::endl;
		assert(_draw_class != nullptr);
		return 1;
	}


    VkImageCreateInfo _imageCreateInfo = {};
    _imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    _imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    _imageCreateInfo.format = VK_FORMAT_R32_UINT;
    _imageCreateInfo.extent.width = SmoothieCore::getScrWidth();
    _imageCreateInfo.extent.height = SmoothieCore::getScrHeight();
    _imageCreateInfo.extent.depth = 1;
    _imageCreateInfo.mipLevels = 1;
    _imageCreateInfo.arrayLayers = 1;
    _imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    _imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    _imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    _imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    _imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo _allocationCreateInfo = {};
    _allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    VkImageViewCreateInfo _imageViewCreateInfo = {};
    _imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    _imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    _imageViewCreateInfo.format = _imageCreateInfo.format;
    _imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    _imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    _imageViewCreateInfo.subresourceRange.layerCount = 1;
    _imageViewCreateInfo.subresourceRange.levelCount = 1;
    _imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    _imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    _imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    _imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;

	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_ModelIDsImage, m_ModelIDsImageAllocation);
	m_ModelIDsImage = nullptr, m_ModelIDsImageAllocation = nullptr;
    if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_imageCreateInfo, &_allocationCreateInfo, &m_ModelIDsImage, &m_ModelIDsImageAllocation, nullptr) != VK_SUCCESS)
    {
        std::cout << "Failed to create Model ids image!" << std::endl;
        return 1;
    }

	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_InstanceIDsImage, m_InstanceIDsImageAllocation);
	m_InstanceIDsImage = nullptr, m_InstanceIDsImageAllocation = nullptr;
    if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_imageCreateInfo, &_allocationCreateInfo, &m_InstanceIDsImage, &m_InstanceIDsImageAllocation, nullptr) != VK_SUCCESS)
    {
        std::cout << "Failed to create instance ids image!" << std::endl;
        return 1;
    }


	vkDestroyImageView(SmoothieCore::getDevice(), m_ModelIDsImageView, nullptr), m_ModelIDsImageView = nullptr;
    _imageViewCreateInfo.image = m_ModelIDsImage;
    if (vkCreateImageView(SmoothieCore::getDevice(), &_imageViewCreateInfo, nullptr, &m_ModelIDsImageView) != VK_SUCCESS)
    {
        std::cout << "Failed to create Model ids view!" << std::endl;
        return 1;
    }

	vkDestroyImageView(SmoothieCore::getDevice(), m_InstanceIDsImageView, nullptr), m_InstanceIDsImageView = nullptr;
    _imageViewCreateInfo.image = m_InstanceIDsImage;
    if (vkCreateImageView(SmoothieCore::getDevice(), &_imageViewCreateInfo, nullptr, &m_InstanceIDsImageView) != VK_SUCCESS)
    {
        std::cout << "Failed to create instance ids view!" << std::endl;
        return 1;
    }


	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_pickDepthImage, m_pickDepthImageAllocation);
	m_pickDepthImage = nullptr, m_pickDepthImageAllocation = nullptr;
    _imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    _imageCreateInfo.format = VK_FORMAT_D32_SFLOAT;
    if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_imageCreateInfo, &_allocationCreateInfo, &m_pickDepthImage, &m_pickDepthImageAllocation, nullptr) != VK_SUCCESS)
    {
        std::cout << "Failed to create Model ids image!" << std::endl;
        return 1;
    }

	vkDestroyImageView(SmoothieCore::getDevice(), m_pickDepthImageView, nullptr), m_pickDepthImageView = nullptr;
    _imageViewCreateInfo.image = m_pickDepthImage;
    _imageViewCreateInfo.format = _imageCreateInfo.format;
    _imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (vkCreateImageView(SmoothieCore::getDevice(), &_imageViewCreateInfo, nullptr, &m_pickDepthImageView) != VK_SUCCESS)
    {
        std::cout << "Failed to create instance ids view!" << std::endl;
        return 1;
    }

	VkSampler _sampler = _draw_class->getSampler("Texture2DModelSampler");
	if (_sampler == nullptr)
	{
		std::cout << "Failed to get the sampler!" << std::endl;
		assert(_sampler != nullptr);
		return 1;
	}

    for (auto& _buffer: m_buffers)
	{

		VkDescriptorBufferInfo _bufferInfo = {};
		_bufferInfo.buffer = _buffer.buffer;
		_bufferInfo.offset = 0;
		_bufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = _buffer.descriptorSet;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &_bufferInfo;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);

		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.pBufferInfo = nullptr;
		VkDescriptorImageInfo imgInfo{};
		imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgInfo.sampler = _sampler;

    	imgInfo.imageView = _draw_class->getHdrPass().gDepth.getImageView();
		descriptorWrite.dstBinding = 1;
		descriptorWrite.pImageInfo = &imgInfo;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);

    	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    	descriptorWrite.pBufferInfo = nullptr;
    	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    	imgInfo.sampler = nullptr;

    	imgInfo.imageView = m_ModelIDsImageView;
    	descriptorWrite.dstBinding = 2;
    	descriptorWrite.pImageInfo = &imgInfo;
    	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);

    	imgInfo.imageView = m_InstanceIDsImageView;
    	descriptorWrite.dstBinding = 3;
    	descriptorWrite.pImageInfo = &imgInfo;
    	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	return 0;
}

void Tool_ScreenPicker::destroy()
{

	vkDestroyPipeline(SmoothieCore::getDevice(), m_sampleDataPipline, nullptr), m_sampleDataPipline = nullptr;
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), m_sampleDataPipelineLayout, nullptr), m_sampleDataPipelineLayout = nullptr;
	for (auto& _buffer : m_buffers)
	{
		vkDestroyDescriptorPool(SmoothieCore::getDevice(), _buffer.descriptorPool, nullptr);
		_buffer.descriptorPool = nullptr, _buffer.descriptorSet = nullptr;
		vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), _buffer.buffer, _buffer.allocation);
		_buffer.buffer = nullptr, _buffer.allocation = nullptr;
	}
	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), m_descriptorSetLayout, nullptr);
	m_descriptorSetLayout = nullptr;

    vkDestroyImageView(SmoothieCore::getDevice(), m_pickDepthImageView, nullptr), m_pickDepthImageView = nullptr;
    vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_pickDepthImage, m_pickDepthImageAllocation), m_pickDepthImage = nullptr, m_pickDepthImageAllocation = nullptr;

    vkDestroyImageView(SmoothieCore::getDevice(), m_InstanceIDsImageView, nullptr), m_InstanceIDsImageView = nullptr;
    vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_InstanceIDsImage, m_InstanceIDsImageAllocation), m_InstanceIDsImage = nullptr, m_InstanceIDsImageAllocation = nullptr;

    vkDestroyImageView(SmoothieCore::getDevice(), m_ModelIDsImageView, nullptr), m_ModelIDsImageView = nullptr;
    vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_ModelIDsImage, m_ModelIDsImageAllocation), m_ModelIDsImage = nullptr, m_ModelIDsImageAllocation = nullptr;

	m_pCore.reset();
}