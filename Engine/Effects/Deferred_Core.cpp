#include "Deferred_Core.h"

#include <iostream>
#include <mutex>
#include <array>
#include <assert.h>

#include "Core/SmoothieCore.h"
#include "Core/Constants.h"

using namespace Smoothie;

int Smoothie::DeferredRendering::Drawing::create_samplers()
{
	VkSamplerCreateInfo Texture2DModelSamplerInfo{};
	Texture2DModelSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	Texture2DModelSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	Texture2DModelSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	Texture2DModelSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	Texture2DModelSamplerInfo.minFilter = VK_FILTER_NEAREST;
	Texture2DModelSamplerInfo.magFilter = VK_FILTER_LINEAR;
	Texture2DModelSamplerInfo.minLod = 0.0f;
	Texture2DModelSamplerInfo.maxLod = SMOOTHIE_IMAGE_MAX_LOD_MODEL_IMAGE;
	Texture2DModelSamplerInfo.unnormalizedCoordinates = false;
	Texture2DModelSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	Texture2DModelSamplerInfo.unnormalizedCoordinates = false;
	VkSampler Texture2DModelSampler = nullptr;
	vkCreateSampler(SmoothieCore::getDevice(), &Texture2DModelSamplerInfo, nullptr, &Texture2DModelSampler);
	samplers["Texture2DModelSampler"] = Texture2DModelSampler;

	VkSamplerCreateInfo _ClampToEdgeLINEAR{};
	_ClampToEdgeLINEAR.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	_ClampToEdgeLINEAR.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	_ClampToEdgeLINEAR.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	_ClampToEdgeLINEAR.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	_ClampToEdgeLINEAR.minFilter = VK_FILTER_LINEAR;
	_ClampToEdgeLINEAR.magFilter = VK_FILTER_LINEAR;
	_ClampToEdgeLINEAR.minLod = 0.0f;
	_ClampToEdgeLINEAR.maxLod = SMOOTHIE_IMAGE_MAX_LOD_MODEL_IMAGE;
	_ClampToEdgeLINEAR.unnormalizedCoordinates = false;
	_ClampToEdgeLINEAR.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	VkSampler ClampToEdgeLINEAR = nullptr;
	vkCreateSampler(SmoothieCore::getDevice(), &_ClampToEdgeLINEAR, nullptr, &ClampToEdgeLINEAR);
	samplers["ClampToEdgeLINEAR"] = ClampToEdgeLINEAR;

	VkSamplerCreateInfo _ClampToEdgeNEAREST = _ClampToEdgeLINEAR;
	_ClampToEdgeNEAREST.minFilter = VK_FILTER_NEAREST;
	_ClampToEdgeNEAREST.magFilter = VK_FILTER_NEAREST;
	_ClampToEdgeNEAREST.maxLod = 0.0f;
	VkSampler ClampToEdgeNEAREST = nullptr;
	vkCreateSampler(SmoothieCore::getDevice(), &_ClampToEdgeNEAREST, nullptr, &ClampToEdgeNEAREST);
	samplers["ClampToEdgeNEAREST"] = ClampToEdgeNEAREST;

	VkSamplerCreateInfo _RepeatNEAREST = Texture2DModelSamplerInfo;
	_RepeatNEAREST.maxLod = 0.0f;
	VkSampler RepeatNEAREST = nullptr;
	vkCreateSampler(SmoothieCore::getDevice(), &_RepeatNEAREST, nullptr, &RepeatNEAREST);
	samplers["RepeatNEAREST"] = RepeatNEAREST;

	return 0;
}

int Smoothie::DeferredRendering::Drawing::create_present_pipeline()
{
	std::array<VkImageView, 2> images =
	{
		hdrPass.HDR.getImageView(),
		bloom.getFinalImage()
	};
	
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayouts(images.size());
	for (int i = 0; i < images.size(); i++)
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = i;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBinding.pImmutableSamplers = nullptr;
		descriptorSetLayouts[i] = layoutBinding;
	}

	//Descriptor set layout
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<unsigned int>(descriptorSetLayouts.size());
	layoutInfo.pBindings = descriptorSetLayouts.data();
	vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &layoutInfo, nullptr, &presentDescriptorSetLayout);
	
	VkDescriptorPoolSize texturePoolSize{};
	texturePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	texturePoolSize.descriptorCount = static_cast<unsigned int>(images.size());
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &texturePoolSize;
	poolInfo.maxSets = 1;
	vkCreateDescriptorPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &presentDescriptorPool);

	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = presentDescriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &presentDescriptorSetLayout;
	vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &presentDescriptorSet);
	for (int i = 0; i < images.size(); i++)
	{
		VkDescriptorImageInfo imgInfo{};
		imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgInfo.imageView = images[i];
		imgInfo.sampler = this->getSampler("Texture2DModelSampler");

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = presentDescriptorSet;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.dstBinding = i;
		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imgInfo;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
	}
	

	const VkDescriptorSetLayout descriptorSets[] =
	{
		SmoothieCore::getCameraDescriptorSetLayout(0),
		this->getDescriptorSetLayout(0),
		presentDescriptorSetLayout
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = sizeof(descriptorSets) / sizeof(descriptorSets[0]);
	pipelineLayoutInfo.pSetLayouts = descriptorSets;
	if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &presentPipelineLayout) != 0)
	{
		std::cout << "Failed to create pipeline layout!" << std::endl;
		return 1;
	}


	VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
	vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderPipelineCreateInfo.pName = "vertex_QUAD";
	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderPipelineCreateInfo.module = getSystemShaderModule("vertex_default");

	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderPipelineCreateInfo.pName = "fragment_PRESENT";
	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderPipelineCreateInfo.module = getSystemShaderModule("present");

	const VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;
	Smoothie::DefaultPipelineState __state;
	__state.populate_pipeline(pipelineInfo);
	pipelineInfo.renderPass = SmoothieCore::getDefaultRenderPass();
	pipelineInfo.layout = presentPipelineLayout;
	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), nullptr, 1, &pipelineInfo, nullptr, &presentPipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create global illumination pipeline!" << std::endl;
		return 1;
	}

	return 0;
}

void Smoothie::DeferredRendering::Drawing::destroy_present_pipeline()
{
	vkDestroyPipeline(SmoothieCore::getDevice(), presentPipeline, nullptr), presentPipeline = nullptr;
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), presentPipelineLayout, nullptr), presentPipelineLayout = nullptr;
	vkDestroyDescriptorPool(SmoothieCore::getDevice(), presentDescriptorPool, nullptr);
	presentDescriptorPool = nullptr, presentDescriptorSet = nullptr;
	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), presentDescriptorSetLayout, nullptr), presentDescriptorSetLayout = nullptr;
}

int Smoothie::DeferredRendering::Drawing::create_system_shaders()
{
	for (const auto& [name, shader]: m_InitInfo.system_shader_files)
	{
		ShaderFile _shaderFile;
		if (_shaderFile.create(shader, ShaderFile_CreateFlags::DontCreateModule | ShaderFile_CreateFlags::Hold_SPIR_V))
		{
			std::cout << "Failed to create system shader file!" << std::endl;
			return 1;
		}
		VkShaderModule _shaderModule = nullptr;
		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = _shaderFile.get_SPIR_V_CODE().size() * sizeof(uint32_t);
		assert(shaderModuleCreateInfo.codeSize != 0);
		shaderModuleCreateInfo.pCode = _shaderFile.get_SPIR_V_CODE().data();
		assert(shaderModuleCreateInfo.pCode != nullptr);
		shaderModuleCreateInfo.pNext = nullptr;
		shaderModuleCreateInfo.flags = 0;
		if (vkCreateShaderModule(SmoothieCore::getDevice(), &shaderModuleCreateInfo, nullptr, &_shaderModule) != VK_SUCCESS)
		{
			std::cout << "Failed to create system shader module!" << std::endl;
			return 1;
		}
		m_SystemShaders.insert({name, _shaderModule});
	}

	return 0;
}

void Smoothie::DeferredRendering::Drawing::destroy_system_shaders()
{
	for (auto& [key, value]: m_SystemShaders)
	{
		vkDestroyShaderModule(SmoothieCore::getDevice(), value, nullptr), value = nullptr;
	}
	m_SystemShaders.clear();
}

int DeferredRendering::Drawing::create_command_buffers()
{

	m_DrawingBuffers.resize(SMOOTHIE_MAX_FRAMES_IN_FLIGHT);
	for (auto& _data: m_DrawingBuffers)
	{
		VkCommandPoolCreateInfo _poolCreateInfo{};
		_poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		_poolCreateInfo.pNext = nullptr;
		_poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		_poolCreateInfo.queueFamilyIndex = SmoothieCore::getQueueFamilyGraphicsIndex();
		if (vkCreateCommandPool(SmoothieCore::getDevice(), &_poolCreateInfo, nullptr, &_data.commandPool) != VK_SUCCESS)
		{
			std::cout << "Failed to create drawing command pool!" << std::endl;
			return 1;
		}

		VkCommandBufferAllocateInfo _commandBufferAllocateInfo{};
		_commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		_commandBufferAllocateInfo.commandPool = _data.commandPool;
		_commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

		VkCommandBuffer _commandBuffers[] = {_data.commandBuffer_GBuffer, _data.commandBuffer_HDR, _data.commandBuffer_Shadow, _data.commandBuffer_Compute};
		_commandBufferAllocateInfo.commandBufferCount = 4;
		if (vkAllocateCommandBuffers(SmoothieCore::getDevice(), &_commandBufferAllocateInfo, _commandBuffers) != VK_SUCCESS)
		{
			std::cout << "Failed to allocate command buffers!" << std::endl;
			return 1;
		}
		_data.commandBuffer_GBuffer = _commandBuffers[0];
		_data.commandBuffer_HDR = _commandBuffers[1];
		_data.commandBuffer_Shadow = _commandBuffers[2];
		_data.commandBuffer_Compute = _commandBuffers[3];

	}

	return 0;
}

void DeferredRendering::Drawing::destroy_command_buffers()
{
	for (auto& _data: m_DrawingBuffers)
	{
		vkDestroyCommandPool(SmoothieCore::getDevice(), _data.commandPool, nullptr), _data.commandPool = nullptr;
		_data.commandBuffer_GBuffer = nullptr;
		_data.commandBuffer_HDR = nullptr;
		_data.commandBuffer_Shadow = nullptr;
		_data.commandBuffer_Compute = nullptr;

	}
}

void DeferredRendering::Drawing::begin_command_buffers(unsigned int frame) const
{
	const auto& _data = m_DrawingBuffers[frame];

	VkCommandBufferBeginInfo _beginInfo{};
	_beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	_beginInfo.pNext = nullptr;
	_beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkCommandBufferInheritanceInfo _inheritanceInfo{};
	_inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	_inheritanceInfo.pNext = nullptr;
	_inheritanceInfo.subpass = 0;

	_inheritanceInfo.framebuffer = gBuffer_Pass.get_framebuffer();
	_inheritanceInfo.renderPass = gBuffer_Pass.get_render_pass();
	_beginInfo.pInheritanceInfo = &_inheritanceInfo;
	vkBeginCommandBuffer(_data.commandBuffer_GBuffer, &_beginInfo);

	_inheritanceInfo.framebuffer = hdrPass.get_framebuffer();
	_inheritanceInfo.renderPass = hdrPass.get_render_pass();
	_beginInfo.pInheritanceInfo = &_inheritanceInfo;
	vkBeginCommandBuffer(_data.commandBuffer_HDR, &_beginInfo);

	//vkBeginCommandBuffer(m_commandBuffer_Shadow, &_beginInfo);

	_beginInfo.flags = 0;
	_beginInfo.pInheritanceInfo = &_inheritanceInfo;
	vkBeginCommandBuffer(_data.commandBuffer_Compute, &_beginInfo);
}

void DeferredRendering::Drawing::end_command_buffers(unsigned int frame) const
{
	const auto& _data = m_DrawingBuffers[frame];
	vkEndCommandBuffer(_data.commandBuffer_GBuffer);
	vkEndCommandBuffer(_data.commandBuffer_HDR);
	//vkEndCommandBuffer(m_commandBuffer_Shadow);
	vkEndCommandBuffer(_data.commandBuffer_Compute);
}

int Smoothie::DeferredRendering::Drawing::create()
{
	if (create_command_buffers())
	{
		std::cout << "Failed to create draw command buffer data!" << std::endl;
		return 1;
	}

	create_samplers();
	descriptors.resize(SMOOTHIE_MAX_FRAMES_IN_FLIGHT);
	if (create_system_shaders() != 0)
	{
		std::cout << "Failed to create system shaders!" << std::endl;
		return 1;
	}

	for (size_t i = 0; i < descriptors.size(); i++)
	{
		if (descriptors[i].create() != 0)
		{
			std::cout << "Failed to create descriptor set " << i << std::endl; 
			return 1;
		}
		descriptors[i].update_descriptor_data(descriptorSetData);
	}

	gBuffer_Pass.sampler = getSampler("Texture2DModelSampler");
	if (gBuffer_Pass.create() != 0)
	{
		std::cout << "Failed to create gBuffer pass!" << std::endl;
		return 1;
	}

	hdrPass.gDepth = gBuffer_Pass.gDepth;

	if (hdrPass.create() != 0)
	{
		std::cout << "Failed to create HDR pass!" << std::endl;
		return 1;
	}

	lighting_renderPass.target = hdrPass.HDR;
	if (lighting_renderPass.create() != 0)
	{
		std::cout << "Failed to create lighting render pass!" << std::endl;
		return 1;
	}
	brdf.Texture2DModelSampler = getSampler("Texture2DModelSampler");
	brdf.vertexShader = getSystemShaderModule("vertex_default");
	brdf.fragmentShader = getSystemShaderModule("pbs_maps");
	if (brdf.create() != 0)
	{
		std::cout << "Failed to create BRDF LUT!" << std::endl;
		return 1;
	}

	skyboxTexture.Texture2DModelSampler = getSampler("Texture2DModelSampler");
	skyboxTexture.m_FragmentShaderModule = getSystemShaderModule("hdr_to_cubemap");
	skyboxTexture.m_VertexShaderModule = getSystemShaderModule("pbs_maps");
	if (skyboxTexture.create() != 0)
	{
		std::cout << "Failed to create skybox image!" << std::endl;
		return 1;
	}
	if (skyboxTexture.create_from_hdri_image("resources/DemoScene/glencairn_expressway_4k.hdr") != 0)
	{

		return 1;
	}

	indirect_lighting_maps.Texture2DModelSampler = getSampler("Texture2DModelSampler");
	indirect_lighting_maps.m_ShaderModule = getSystemShaderModule("pbs_maps");

	if (indirect_lighting_maps.create() != 0)
	{
		std::cout << "Failed to create maps for global illumination!" << std::endl;
		return 1;
	}
	indirect_lighting_maps.create_maps_from_skybox_cubemap(skyboxTexture);

	ssao.gDepthImageView = gBuffer_Pass.gDepth.getImageView();
	ssao.gNormalImageView = gBuffer_Pass.gNormal.getImageView();
	if (ssao.create() != 0)
	{
		std::cout << "Failed to create SSAO!" << std::endl;
		return 1;
	}

	lighting_global.gBufferImagesDescriptorSet = gBuffer_Pass.getImagesDescriptorSet();
	lighting_global.gBufferImagesDescriptorSetLayout = gBuffer_Pass.getImagesDescriptorSetLayout();
	lighting_global.BRDF_DescriptorSet = brdf.getDescriptorSet();
	lighting_global.BRDF_DescriptorSetLayout = brdf.getDescriptorSetLayout();
	lighting_global.IrradiancePrefilter_DescriptorSet = indirect_lighting_maps.getDescriptorSet();
	lighting_global.IrradiancePrefilter_DescriptorSetLayout = indirect_lighting_maps.getDescriptorSetLayout();
	lighting_global.DrawClassSetLayout = getDescriptorSetLayout(0);
	lighting_global.renderPass = hdrPass.get_render_pass();
	lighting_global.vertexShader = getSystemShaderModule("vertex_default");
	lighting_global.globalIlluminationModule = getSystemShaderModule("pbr_render");
	if (lighting_global.create() != 0)
	{
		std::cout << "Failed to create global lighting!" << std::endl;
		return 1;
	}
	
	skybox.HDRCubemap_descriptorSet = skyboxTexture.getDescriptorSet();
	skybox.HDRCubemap_descriptorSetLayout = skyboxTexture.getDescriptorSetLayout();
	skybox.renderPass = hdrPass.get_render_pass();
	skybox.drawerClassDescriptorSetLayout = getDescriptorSetLayout(0);
	skybox.m_ShaderModule = getSystemShaderModule("skybox");
	if (skybox.create() != 0)
	{
		std::cout << "Failed to create skybox!" << std::endl;
		return 1;
	}

	bloom.HDRImageView = hdrPass.HDR.getImageView();
	bloom.HDRImage = hdrPass.HDR.getImage();
	bloom.ClampToEdgeLINEAR = getSampler("ClampToEdgeLINEAR");
	bloom.vertexShader = getSystemShaderModule("vertex_default");
	bloom.m_FragmentShader = getSystemShaderModule("bloom");
	if (bloom.create() != 0)
	{
		std::cout << "Failed to create bloom effect!" << std::endl;
		return 1;
	}

	if (create_present_pipeline() != 0)
	{
		std::cout << "Failed to create present pipeline!" << std::endl;
		return 1;
	}

    return 0;
}

void Smoothie::DeferredRendering::Drawing::draw(VkCommandBuffer commandBuffer, unsigned int imageIndex) const
{
	const auto& _draw_data_buffers = m_DrawingBuffers[SmoothieCore::getCurrentFrame()];
	const VkDescriptorSet _descriptorSet = getDescriptorSet();
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0,
		0, nullptr,
		0, nullptr,
		0, nullptr);

	begin_command_buffers(SmoothieCore::getCurrentFrame());
	RecordCommandsData _draw_data;
	_draw_data.commandBuffer_gBuffer = _draw_data_buffers.commandBuffer_GBuffer;
	_draw_data.commandBuffer_HDR = _draw_data_buffers.commandBuffer_HDR;
	_draw_data.commandBuffer_Compute = _draw_data_buffers.commandBuffer_Compute;
	_draw_data.commandBuffer_Shadow = _draw_data_buffers.commandBuffer_Shadow;
	_draw_data.descriptorSet_DrawClass = _descriptorSet;
	_draw_data.imageIndex = imageIndex;
	for (const auto& _pipe : m_ModelPipelines)
	{
		_pipe->record_commands(_draw_data);
	}

	lighting_global.draw(_draw_data.commandBuffer_HDR, _descriptorSet, imageIndex);
	skybox.draw(_draw_data.commandBuffer_HDR, _descriptorSet, imageIndex);
	end_command_buffers(SmoothieCore::getCurrentFrame());

	//G-Buffer pass
	gBuffer_Pass.bindPass(commandBuffer, imageIndex);
	vkCmdExecuteCommands(commandBuffer, 1, &_draw_data_buffers.commandBuffer_GBuffer);
	gBuffer_Pass.unbindPass(commandBuffer, imageIndex);
	
	//Screen space lighting effects
	ssao.draw(commandBuffer, _descriptorSet, imageIndex);

	//HDR pass
	hdrPass.bindPass(commandBuffer, imageIndex);
	vkCmdExecuteCommands(commandBuffer, 1, &_draw_data_buffers.commandBuffer_HDR);
	hdrPass.unbindPass(commandBuffer, imageIndex);

	//Post-processing effects
	bloom.draw(commandBuffer, _descriptorSet, imageIndex);

	//If editor class exists, that class will handle the presentation
	if (get_editor_core() != nullptr) return;

	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.renderPass = SmoothieCore::getDefaultRenderPass();
	beginInfo.framebuffer = SmoothieCore::getSwapchainFramebuffer(imageIndex);
	beginInfo.renderArea.offset = { 0, 0 };
	beginInfo.renderArea.extent.height = SmoothieCore::getScrHeight();
	beginInfo.renderArea.extent.width = SmoothieCore::getScrWidth();
	VkClearValue clearValue{};
	clearValue.depthStencil = { 0.0f, 0 };
	clearValue.color.float32[0] = 0.0f;
	clearValue.color.float32[1] = 0.0f;
	clearValue.color.float32[2] = 0.0f;
	clearValue.color.float32[3] = 1.0f;
	beginInfo.clearValueCount = 1;
	beginInfo.pClearValues = &clearValue;
	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, presentPipeline);
	SmoothieCore::setViewport(commandBuffer);
	SmoothieCore::setScissor(commandBuffer);

	const VkDescriptorSet _descriptors[3] =
	{
		SmoothieCore::getCameraDescriptorSet(),
		_descriptorSet,
		presentDescriptorSet
	};
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, presentPipelineLayout, 0, 3, _descriptors, 0, 0);
	vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);
}

int Smoothie::DeferredRendering::Drawing::resize_callback()
{
	if (gBuffer_Pass.resize_callback() != 0)
	{
		std::cout << "Failed to resize gBuffer pass!" << std::endl;
		return 1;
	}

	hdrPass.gDepth = gBuffer_Pass.gDepth;
	if (hdrPass.resize_callback() != 0)
	{
		std::cout << "Failed to resize HDR pass!" << std::endl;
		return 1;
	}

	lighting_renderPass.target = hdrPass.HDR;
	if (lighting_renderPass.resize_callback() != 0)
	{
		std::cout << "Failed to resize lighting render pass!" << std::endl;
		return 1;
	}

	ssao.gDepthImageView = gBuffer_Pass.gDepth.getImageView();
	ssao.gNormalImageView = gBuffer_Pass.gNormal.getImageView();
	if (ssao.resize_callback() != 0)
	{
		std::cout << "Failed to resize SSAO!" << std::endl;
		return 1;
	}

	bloom.HDRImageView = hdrPass.HDR.getImageView();
	bloom.HDRImage = hdrPass.HDR.getImage();
	if (bloom.resize_callback() != 0)
	{
		std::cout << "Failed to resize bloom effect!" << std::endl;
		return 1;
	}

	//Update last render pass
	std::array<VkImageView, 2> images =
	{
		hdrPass.HDR.getImageView(),
		bloom.getFinalImage()
	};

	for (int i = 0; i < images.size(); i++)
	{
		VkDescriptorImageInfo imgInfo{};
		imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgInfo.imageView = images[i];
		imgInfo.sampler = this->getSampler("Texture2DModelSampler");

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = presentDescriptorSet;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.dstBinding = i;
		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imgInfo;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	return 0;
}

void Smoothie::DeferredRendering::Drawing::destroy()
{
	for (auto& _pipe: m_ModelPipelines)
	{
		if (_pipe != nullptr) _pipe->destroy();
	}
	m_ModelPipelines.clear();


	destroy_present_pipeline();
	bloom.destroy();
	skybox.destroy();
	lighting_global.destroy();
	ssao.destroy();
	indirect_lighting_maps.destroy();
	skyboxTexture.destroy();
	brdf.destroy();
	lighting_renderPass.destroy();
	hdrPass.destroy();
	gBuffer_Pass.destroy();

	for (auto& _descriptor: descriptors)
	{
		_descriptor.destroy();
	}
	descriptors.clear();

	for (auto& [key, value]: samplers)
	{
		vkDestroySampler(SmoothieCore::getDevice(), value, nullptr);
		value = nullptr;
	}
	destroy_system_shaders();
	samplers.clear();
	destroy_command_buffers();
}

static std::mutex add_or_create_pipeline_lock;
VkDescriptorSet Smoothie::DeferredRendering::Drawing::getDescriptorSet() const
{
	return descriptors[SmoothieCore::getCurrentFrame()].getDescriptorSet();
}

VkSampler Smoothie::DeferredRendering::Drawing::getSampler(const std::string& name) const
{
	if (samplers.find(name) != samplers.end()) 
	{ 
		return samplers.at(name); 
	} 
	return nullptr;
}

VkShaderModule Smoothie::DeferredRendering::Drawing::getSystemShaderModule(const std::string& identifier) const
{
	if (m_SystemShaders.find(identifier) != m_SystemShaders.end())
	{
		return m_SystemShaders.at(identifier);
	}
	return nullptr;
}

int DeferredRendering::Drawing::add_model(const std::shared_ptr<BasePipeline>& pipeline, std::shared_ptr<Deferred_Model> &model)
{
	std::lock_guard<std::mutex> lock(add_or_create_pipeline_lock);
	for (auto& _pipe: m_ModelPipelines)
	{
		if (_pipe != nullptr && _pipe == pipeline)
		{
			return _pipe->add_to_rendering(model);
		}
	}

	m_ModelPipelines.push_back(pipeline);
	return pipeline->add_to_rendering(model);
}

void DeferredRendering::Drawing::remove_model(const std::shared_ptr<BasePipeline>& pipeline, std::shared_ptr<Deferred_Model> &model)
{
	std::lock_guard<std::mutex> lock(add_or_create_pipeline_lock);
	for (auto& _pipe: m_ModelPipelines)
	{
		if (_pipe != nullptr && _pipe == pipeline)
		{
			_pipe->remove_from_rendering(model);
		}
	}
}

void Smoothie::DeferredRendering::Drawing::update_descriptorSet()
{
	descriptors[SmoothieCore::getCurrentFrame()].update_descriptor_data(descriptorSetData);
}

