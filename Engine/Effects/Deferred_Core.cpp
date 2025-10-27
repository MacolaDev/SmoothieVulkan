#include "Deferred_Core.h"

#include <iostream>
#include <mutex>
#include <array>

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
	vertexShaderPipelineCreateInfo.pName = "main";
	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderPipelineCreateInfo.module = getSystemShaderModule("SYSTEM");

	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderPipelineCreateInfo.pName = "main";
	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderPipelineCreateInfo.module = getSystemShaderModule("PRESENT");

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

int Smoothie::DeferredRendering::Drawing::create_system_shaders(const std::vector<const char*>& __shader_files)
{
	for (size_t i = 0; i < __shader_files.size(); i++)
	{
		if (Add_system_shader(__shader_files[i], system_shaders) != 0)
		{
			std::cout << "Failed to create shader module!" << std::endl;
			return 1;
		}
	}

	return 0;
}

void Smoothie::DeferredRendering::Drawing::destroy_system_shaders()
{
	for (auto& [key, value]: system_shaders)
	{
		vkDestroyShaderModule(SmoothieCore::getDevice(), value, nullptr), value = nullptr;
	}
	system_shaders.clear();
}

int Smoothie::DeferredRendering::Drawing::create()
{
	create_samplers();
	descriptors.resize(SMOOTHIE_MAX_FRAMES_IN_FLIGHT);
	if (create_system_shaders(__system_shader_files) != 0)
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
	brdf.vertexShader = getSystemShaderModule("SYSTEM");
	brdf.fragmentShader = getSystemShaderModule("BRDF");
	if (brdf.create() != 0)
	{
		std::cout << "Failed to create BRDF LUT!" << std::endl;
		return 1;
	}

	skyboxTexture.Texture2DModelSampler = getSampler("Texture2DModelSampler");
	skyboxTexture.pbsVertexShader = getSystemShaderModule("PBS_VERTEX");
	skyboxTexture.fragmentShader = getSystemShaderModule("HDR_TO_CUBEMAP");

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
	indirect_lighting_maps.pbsVertexModule = getSystemShaderModule("PBS_VERTEX");
	indirect_lighting_maps.irradianceMapModule = getSystemShaderModule("IRRADIANCE_MAP");
	indirect_lighting_maps.prefilterMapModule = getSystemShaderModule("PREFILTER");

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
	lighting_global.vertexShader = getSystemShaderModule("SYSTEM");
	lighting_global.globalIlluminationModule = getSystemShaderModule("GLOBAL_ILLUMINATION");
	if (lighting_global.create() != 0)
	{
		std::cout << "Failed to create global lighting!" << std::endl;
		return 1;
	}
	
	skybox.HDRCubemap_descriptorSet = skyboxTexture.getDescriptorSet();
	skybox.HDRCubemap_descriptorSetLayout = skyboxTexture.getDescriptorSetLayout();
	skybox.renderPass = hdrPass.get_render_pass();
	skybox.drawerClassDescriptorSetLayout = getDescriptorSetLayout(0);
	skybox.skyboxVertex = getSystemShaderModule("SKYBOX_VERTEX");
	skybox.skyboxFragment = getSystemShaderModule("SKYBOX_FRAGMENT");
	if (skybox.create() != 0)
	{
		std::cout << "Failed to create skybox!" << std::endl;
		return 1;
	}

	bloom.HDRImageView = hdrPass.HDR.getImageView();
	bloom.HDRImage = hdrPass.HDR.getImage();
	bloom.ClampToEdgeLINEAR = getSampler("ClampToEdgeLINEAR");
	bloom.vertexShader = getSystemShaderModule("SYSTEM");
	bloom.higlightModule = getSystemShaderModule("BLOOM_HIGLIGHTS");
	bloom.downsampleModule = getSystemShaderModule("BLOOM_DOWNSAMPLE");
	bloom.upsampleModule = getSystemShaderModule("BLOOM_UPSAMPLE");
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

void Smoothie::DeferredRendering::Drawing::draw(VkCommandBuffer commandBuffer, unsigned int currentFrame) const
{
	const VkDescriptorSet descriptorSet = getDescriptorSet();
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0,
		0, nullptr,
		0, nullptr,
		0, nullptr);
	
	//Gbuffer pass
	gBuffer_Pass.bindPass(commandBuffer, currentFrame);
	for (const auto& [key, pipe] : gBufferPipelines)
	{
		const auto* _ptr = pipe.get();
		if (_ptr != nullptr) _ptr->bindAndDraw(commandBuffer, descriptorSet, currentFrame);
	}
	gBuffer_Pass.unbindPass(commandBuffer, currentFrame);
	
	//Screen space lighting effects
	ssao.draw(commandBuffer, descriptorSet, currentFrame);
	
	//HDR pass
	hdrPass.bindPass(commandBuffer, currentFrame);
	lighting_global.draw(commandBuffer, descriptorSet, currentFrame);
	skybox.draw(commandBuffer, descriptorSet, currentFrame);
	for (const auto& [key, pipe] : HDRPipelines)
	{
		if (pipe != nullptr) pipe->bindAndDraw(commandBuffer, descriptorSet, currentFrame);
	}
	hdrPass.unbindPass(commandBuffer, currentFrame);

	//Post processing effects
	bloom.draw(commandBuffer, descriptorSet, currentFrame);


	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.renderPass = SmoothieCore::getDefaultRenderPass();
	beginInfo.framebuffer = SmoothieCore::getSwapchainFramebuffer(currentFrame);
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
		descriptorSet,
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
	for (const auto& [key, pipe]: gBufferPipelines)
	{
		if (pipe != nullptr) pipe->destroy();
	}
	gBufferPipelines.clear();

	for (const auto& [key, pipe] : HDRPipelines)
	{
		if (pipe != nullptr) pipe->destroy();
	}
	HDRPipelines.clear();


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

	for (size_t i = 0; i < descriptors.size(); i++)
	{
		descriptors[i].destroy();
	}
	descriptors.clear();

	for (auto& [key, value]: samplers)
	{
		vkDestroySampler(SmoothieCore::getDevice(), value, nullptr);
		value = nullptr;
	}
	destroy_system_shaders();
	samplers.clear();
}

static std::mutex add_or_create_pipeline_lock;
VkDescriptorSet Smoothie::DeferredRendering::Drawing::getDescriptorSet() const
{
	return descriptors[SmoothieCore::getCurrentFrame()].getDescriptrotSet();
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
	if (system_shaders.find(identifier) != system_shaders.end())
	{
		return system_shaders.at(identifier);
	}
	return nullptr;
}

DeferredRendering::BasePipeline* Smoothie::DeferredRendering::Drawing::get_or_create_pipeline(const std::string& filePath)
{
	//std::lock_guard<std::mutex> lock(add_or_create_pipeline_lock);
	//if (gBufferPipelines.find(filePath) != gBufferPipelines.end())
	//{
	//	return gBufferPipelines[filePath].get();
	//}

	//if (HDRPipelines.find(filePath) != HDRPipelines.end())
	//{
	//	return HDRPipelines[filePath].get();
	//}

	//ShaderFile shaderFile;
	//if (shaderFile.create(filePath) != 0)
	//{
	//	std::cout << "Failed to create ShaderFile!" << std::endl;
	//	return nullptr;
	//}

	//const auto& _filepath = shaderFile.get_filepath();
	//const auto& _shaders = shaderFile.shaders;

	////Standard pipeline :D
	//if (_shaders.find({"STANDARD", VK_SHADER_STAGE_FRAGMENT_BIT}) != _shaders.end())
	//{
	//	const auto& __shader = _shaders.at({ "STANDARD", VK_SHADER_STAGE_FRAGMENT_BIT });

	//	StaticPipeline __pipeline;
	//	__pipeline.setDrawingClassDescriptrSetLayout(getDescriptorSetLayout(0));
	//	__pipeline.setShaderFile(shaderFile);
	//	if (__shader.pipe_outputs.size() == 1)
	//	{
	//		__pipeline.setRenderPass(HDRPass.get_render_pass());
	//		if (__pipeline.create() != 0)
	//		{
	//			std::cout << "Failed to create pipleine from shader file: " << shaderFile.get_filepath() << std::endl;
	//			return nullptr;
	//		}

	//		std::lock_guard<std::mutex> lock(_HDRMutex);
	//		HDRPipelines[_filepath] = std::make_shared<StaticPipeline>(__pipeline);
	//		return HDRPipelines[_filepath].get();
	//	}
	//	else
	//	{
	//		__pipeline.setRenderPass(gBuffer_Pass.get_render_pass());
	//		__pipeline.setShaderFile(shaderFile);
	//		if (__pipeline.create() != 0)
	//		{
	//			std::cout << "Failed to create pipleine from shader file: " << shaderFile.get_filepath() << std::endl;
	//			return nullptr;
	//		}
	//		std::lock_guard<std::mutex> lock(_gBufferMutex);
	//		gBufferPipelines[_filepath] = std::make_shared<StaticPipeline>(__pipeline);
	//		return gBufferPipelines[_filepath].get();
	//	}
	//	

	//}

	return nullptr;
}

int Smoothie::DeferredRendering::Drawing::add_Model_to_rendering(const std::string& _PipeID, StandardModel& model)
{
	//model.setPipeline(dynamic_cast<StaticPipeline*>(get_or_create_pipeline(_PipeID)));
	//if (model.create() != 0)
	//{
	//	std::cout << "Failed to create the model!" << std::endl;
	//	return 1;
	//}

	//if (gBufferPipelines.find(_PipeID) != gBufferPipelines.end())
	//{
	//	std::lock_guard<std::mutex> lock(_gBufferMutex);
	//	const std::shared_ptr<BasePipeline>& __pipelines = gBufferPipelines[_PipeID];
	//	if (__pipelines != nullptr)
	//	{
	//		__pipelines->add_to_rendering(std::make_shared<StandardModel>(model));
	//		return 0;
	//	}
	//	
	//	std::cout << "Pipeline with ID " << _PipeID << "Became invalid!" << std::endl;
	//	return 1;
	//}

	//if (HDRPipelines.find(_PipeID) != HDRPipelines.end())
	//{
	//	std::lock_guard<std::mutex> lock(_HDRMutex);
	//	const std::shared_ptr<BasePipeline>& __pipelines = HDRPipelines[_PipeID];
	//	if (__pipelines != nullptr)
	//	{
	//		__pipelines->add_to_rendering(std::make_shared<StandardModel>(model));
	//		return 0;
	//	}

	//	std::cout << "Pipeline with ID " << _PipeID << "Became invalid!" << std::endl;
	//	return 1;
	//}

	return 0;
}

void Smoothie::DeferredRendering::Drawing::remove_Model_from_rendering(const std::string& shaderFile, const StandardModel& model)
{
	//if (gBufferPipelines.find(shaderFile) != gBufferPipelines.end())
	//{
	//	const std::shared_ptr<BasePipeline>& __pipelines = gBufferPipelines[shaderFile];
	//	if (__pipelines == nullptr) return;
	//	__pipelines->remove_from_rendering(std::make_shared<StandardModel>(model));
	//	
	//}

	//if (HDRPipelines.find(shaderFile) != HDRPipelines.end())
	//{
	//	const std::shared_ptr<BasePipeline>& __pipelines = HDRPipelines[shaderFile];
	//	if (__pipelines == nullptr) return;
	//	__pipelines->remove_from_rendering(std::make_shared<StandardModel>(model));
	//	
	//}
}

void Smoothie::DeferredRendering::Drawing::update_descriptorSet()
{
	descriptors[SmoothieCore::getCurrentFrame()].update_descriptor_data(descriptorSetData);
}

