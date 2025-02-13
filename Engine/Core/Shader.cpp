#include "Shader.h"
#include <iostream>
#include <fstream>
#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"
#include <filesystem>

Shader::Shader(const std::string& file) :ShaderFile(file)
{
	//general module create info
	VkShaderModuleCreateInfo createModuleInfo{};
	createModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	//General pipeline create info
	VkPipelineShaderStageCreateInfo pipelineShaderCreateInfo{};
	pipelineShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineShaderCreateInfo.pName = "main";
	
	
	//Vertex shader 
	createModuleInfo.codeSize = vertexShaderSPIRV.size() * sizeof(unsigned int);
	createModuleInfo.pCode = vertexShaderSPIRV.data();
	vkCreateShaderModule(SmoothieCore::getDevice(), &createModuleInfo, nullptr, &vertexShaderModule);

	pipelineShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipelineShaderCreateInfo.module = vertexShaderModule;
	allShaderStages.push_back(pipelineShaderCreateInfo);


	//Fragment shader
	createModuleInfo.codeSize = fragmentShaderSPIRV.size() * sizeof(unsigned int);
	createModuleInfo.pCode = fragmentShaderSPIRV.data();
	vkCreateShaderModule(SmoothieCore::getDevice(), &createModuleInfo, nullptr, &fragmentShaderModule);

	pipelineShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipelineShaderCreateInfo.module = fragmentShaderModule;
	allShaderStages.push_back(pipelineShaderCreateInfo);



}

void Shader::destroy()
{
	vkDestroyShaderModule(SmoothieCore::getDevice(), fragmentShaderModule, nullptr);
	fragmentShaderModule = nullptr;
	vkDestroyShaderModule(SmoothieCore::getDevice(), vertexShaderModule, nullptr);
	vertexShaderModule = nullptr;
}


std::vector<VkPipelineShaderStageCreateInfo> Shader::getPipelineShaderStages() const
{
	return allShaderStages;
}


static void getShaderFromDataVertex(const std::string& file, VkShaderModule& module)
{
	ShaderFile shaderFile = ShaderFile(file);
	if (shaderFile.hasVertexShader != true)
	{
		std::cout << __FUNCTION__": Shader file " << shaderFile.file << " does not have vertex shader!" << std::endl;
		return;
	}

	VkShaderModuleCreateInfo createModuleInfo{};
	createModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createModuleInfo.codeSize = shaderFile.vertexShaderSPIRV.size() * sizeof(unsigned int);
	createModuleInfo.pCode = shaderFile.vertexShaderSPIRV.data();
	vkCreateShaderModule(SmoothieCore::getDevice(), &createModuleInfo, nullptr, &module);
}

static void getShaderFromDataFragment(const std::string& file, VkShaderModule& module)
{
	ShaderFile shaderFile = ShaderFile(file);
	if (shaderFile.hasFragmentShader != true)
	{
		std::cout << __FUNCTION__": Shader file " << shaderFile.file << "does not have fragment shader!" << std::endl;
		return;
	}

	VkShaderModuleCreateInfo createModuleInfo{};
	createModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createModuleInfo.codeSize = shaderFile.fragmentShaderSPIRV.size() * sizeof(unsigned int);
	createModuleInfo.pCode = shaderFile.fragmentShaderSPIRV.data();
	vkCreateShaderModule(SmoothieCore::getDevice(), &createModuleInfo, nullptr, &module);
}

static void getShaderFromDataVertexFragment(const std::string& file, VkShaderModule& vertexModule, VkShaderModule& fragmentModule)
{
	ShaderFile shaderFile = ShaderFile(file);
	if (shaderFile.hasFragmentShader != true)
	{
		std::cout << __FUNCTION__": Shader file " << shaderFile.file << "does not have fragment shader!" << std::endl;
		return;
	}

	if (shaderFile.hasVertexShader != true)
	{
		std::cout << __FUNCTION__": Shader file " << shaderFile.file << "does not have vertex shader!" << std::endl;
		return;
	}

	VkShaderModuleCreateInfo createModuleInfo{};
	createModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createModuleInfo.codeSize = shaderFile.vertexShaderSPIRV.size() * sizeof(unsigned int);
	createModuleInfo.pCode = shaderFile.vertexShaderSPIRV.data();
	vkCreateShaderModule(SmoothieCore::getDevice(), &createModuleInfo, nullptr, &vertexModule);

	createModuleInfo.codeSize = shaderFile.fragmentShaderSPIRV.size() * sizeof(unsigned int);
	createModuleInfo.pCode = shaderFile.fragmentShaderSPIRV.data();
	vkCreateShaderModule(SmoothieCore::getDevice(), &createModuleInfo, nullptr, &fragmentModule);
}

VkShaderModule PostProcessingShaders::basicVertexShader = nullptr;
VkShaderModule PostProcessingShaders::PBRShader = nullptr;
VkShaderModule PostProcessingShaders::BRDF = nullptr;
VkShaderModule PostProcessingShaders::cubemapVertex = nullptr;
VkShaderModule PostProcessingShaders::HDRToCubemap = nullptr;
VkShaderModule PostProcessingShaders::Irradiance = nullptr;
VkShaderModule PostProcessingShaders::Prefilter = nullptr;

VkShaderModule PostProcessingShaders::SkyboxVertex = nullptr;
VkShaderModule PostProcessingShaders::SkyboxFragment = nullptr;
VkShaderModule PostProcessingShaders::higlights = nullptr;
VkShaderModule PostProcessingShaders::downsampling = nullptr;
VkShaderModule PostProcessingShaders::upsampling = nullptr;
VkShaderModule PostProcessingShaders::DeferredPipelineFinal = nullptr;

void PostProcessingShaders::create()
{
	getShaderFromDataVertex("shaders/post-processing/vertex1.sshader", basicVertexShader);
	getShaderFromDataFragment("shaders/standard/pbr_render.sshader", PBRShader);
	getShaderFromDataFragment("shaders/standard/brdf.sshader", BRDF);
	getShaderFromDataVertex("shaders/standard/cubemapVertex.sshader", cubemapVertex);
	getShaderFromDataFragment("shaders/standard/hdrToCubemap.sshader", HDRToCubemap);
	getShaderFromDataFragment("shaders/standard/irradianceMap.sshader", Irradiance);
	getShaderFromDataFragment("shaders/standard/prefilter.sshader", Prefilter);
	getShaderFromDataVertexFragment("shaders/standard/skybox.sshader", SkyboxVertex, SkyboxFragment);
	getShaderFromDataFragment("shaders/bloom/higlights.sshader", higlights);
	getShaderFromDataFragment("shaders/bloom/downsampling.sshader", downsampling);
	getShaderFromDataFragment("shaders/bloom/upsampling.sshader", upsampling);
	getShaderFromDataFragment("shaders/post-processing/final.sshader", DeferredPipelineFinal);
}

static void destroyShaderModule(VkShaderModule& shaderModule) 
{
	vkDestroyShaderModule(SmoothieCore::getDevice(), shaderModule, nullptr);
	shaderModule = nullptr;
}

static void destroyShaderModule(VkShaderModule& shaderModuleVertex, VkShaderModule& shaderModuleFragment)
{
	vkDestroyShaderModule(SmoothieCore::getDevice(), shaderModuleVertex, nullptr);
	shaderModuleVertex = nullptr;

	vkDestroyShaderModule(SmoothieCore::getDevice(), shaderModuleFragment, nullptr);
	shaderModuleFragment = nullptr;
}

void PostProcessingShaders::destroy()
{
	destroyShaderModule(DeferredPipelineFinal);
	destroyShaderModule(upsampling);
	destroyShaderModule(downsampling);
	destroyShaderModule(higlights);
	destroyShaderModule(SkyboxVertex, SkyboxFragment);
	destroyShaderModule(Prefilter);
	destroyShaderModule(Irradiance);
	destroyShaderModule(HDRToCubemap);
	destroyShaderModule(cubemapVertex);
	destroyShaderModule(BRDF);
	destroyShaderModule(PBRShader);
	destroyShaderModule(basicVertexShader);
}

void ModelShader::create(const std::string& shaderFile)
{
	filepath = shaderFile;
	if (isAlreadyLoaded(shaderFile))
	{
		auto data = getResourse(shaderFile);
		vertexShaderModule = data->vertexShaderModule;
		fragmentShaderModule = data->fragmentShaderModule;
		increaseReferenceCount(shaderFile);
	}
	else
	{
		getShaderFromDataVertexFragment(filepath, vertexShaderModule, fragmentShaderModule);
		setResource(filepath, *this);
		return;
	}

}

void ModelShader::destroy()
{
	decreaseReferenceCount(filepath);
	if (getReferenceCout(filepath) == 0) 
	{
		destroyShaderModule(vertexShaderModule, fragmentShaderModule);
		removeResource(filepath);
	}
}

ModelShader::ModelShader(const std::string& shaderFile)
{
	create(shaderFile);
}

ShaderModelRenderPass ModelShader::getRenderPass() const
{
	ShaderModelRenderPass pass;
	if (filepath.find("pbr") != std::string::npos)
	{
		pass = ShaderModelRenderPass::gBuffer;
	}
	else
	{
		pass = ShaderModelRenderPass::HDRPass;
	}
	return pass;
}

