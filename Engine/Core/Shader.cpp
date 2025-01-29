#include "Shader.h"

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <iostream>
#include <fstream>
#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"
#include "glslang/Include/glslang_c_interface.h"

static TBuiltInResource InitResources() {
	TBuiltInResource Resources;
	Resources.maxLights = 32;
	Resources.maxClipPlanes = 6;
	Resources.maxTextureUnits = 32;
	Resources.maxTextureCoords = 32;
	Resources.maxVertexAttribs = 64;
	Resources.maxVertexUniformComponents = 4096;
	Resources.maxVaryingFloats = 64;
	Resources.maxVertexTextureImageUnits = 32;
	Resources.maxCombinedTextureImageUnits = 80;
	Resources.maxTextureImageUnits = 32;
	Resources.maxFragmentUniformComponents = 4096;
	Resources.maxDrawBuffers = 32;
	Resources.maxVertexUniformVectors = 128;
	Resources.maxVaryingVectors = 8;
	Resources.maxFragmentUniformVectors = 16;
	Resources.maxVertexOutputVectors = 16;
	Resources.maxFragmentInputVectors = 15;
	Resources.minProgramTexelOffset = -8;
	Resources.maxProgramTexelOffset = 7;
	Resources.maxClipDistances = 8;
	Resources.maxComputeWorkGroupCountX = 65535;
	Resources.maxComputeWorkGroupCountY = 65535;
	Resources.maxComputeWorkGroupCountZ = 65535;
	Resources.maxComputeWorkGroupSizeX = 1024;
	Resources.maxComputeWorkGroupSizeY = 1024;
	Resources.maxComputeWorkGroupSizeZ = 64;
	Resources.maxComputeUniformComponents = 1024;
	Resources.maxComputeTextureImageUnits = 16;
	Resources.maxComputeImageUniforms = 8;
	Resources.maxComputeAtomicCounters = 8;
	Resources.maxComputeAtomicCounterBuffers = 1;
	Resources.maxVaryingComponents = 60;
	Resources.maxVertexOutputComponents = 64;
	Resources.maxGeometryInputComponents = 64;
	Resources.maxGeometryOutputComponents = 128;
	Resources.maxFragmentInputComponents = 128;
	Resources.maxImageUnits = 8;
	Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	Resources.maxCombinedShaderOutputResources = 8;
	Resources.maxImageSamples = 0;
	Resources.maxVertexImageUniforms = 0;
	Resources.maxTessControlImageUniforms = 0;
	Resources.maxTessEvaluationImageUniforms = 0;
	Resources.maxGeometryImageUniforms = 0;
	Resources.maxFragmentImageUniforms = 8;
	Resources.maxCombinedImageUniforms = 8;
	Resources.maxGeometryTextureImageUnits = 16;
	Resources.maxGeometryOutputVertices = 256;
	Resources.maxGeometryTotalOutputComponents = 1024;
	Resources.maxGeometryUniformComponents = 1024;
	Resources.maxGeometryVaryingComponents = 64;
	Resources.maxTessControlInputComponents = 128;
	Resources.maxTessControlOutputComponents = 128;
	Resources.maxTessControlTextureImageUnits = 16;
	Resources.maxTessControlUniformComponents = 1024;
	Resources.maxTessControlTotalOutputComponents = 4096;
	Resources.maxTessEvaluationInputComponents = 128;
	Resources.maxTessEvaluationOutputComponents = 128;
	Resources.maxTessEvaluationTextureImageUnits = 16;
	Resources.maxTessEvaluationUniformComponents = 1024;
	Resources.maxTessPatchComponents = 120;
	Resources.maxPatchVertices = 32;
	Resources.maxTessGenLevel = 64;
	Resources.maxViewports = 16;
	Resources.maxVertexAtomicCounters = 0;
	Resources.maxTessControlAtomicCounters = 0;
	Resources.maxTessEvaluationAtomicCounters = 0;
	Resources.maxGeometryAtomicCounters = 0;
	Resources.maxFragmentAtomicCounters = 8;
	Resources.maxCombinedAtomicCounters = 8;
	Resources.maxAtomicCounterBindings = 1;
	Resources.maxVertexAtomicCounterBuffers = 0;
	Resources.maxTessControlAtomicCounterBuffers = 0;
	Resources.maxTessEvaluationAtomicCounterBuffers = 0;
	Resources.maxGeometryAtomicCounterBuffers = 0;
	Resources.maxFragmentAtomicCounterBuffers = 1;
	Resources.maxCombinedAtomicCounterBuffers = 1;
	Resources.maxAtomicCounterBufferSize = 16384;
	Resources.maxTransformFeedbackBuffers = 4;
	Resources.maxTransformFeedbackInterleavedComponents = 64;
	Resources.maxCullDistances = 8;
	Resources.maxCombinedClipAndCullDistances = 8;
	Resources.maxSamples = 4;
	Resources.maxMeshOutputVerticesNV = 256;
	Resources.maxMeshOutputPrimitivesNV = 512;
	Resources.maxMeshWorkGroupSizeX_NV = 32;
	Resources.maxMeshWorkGroupSizeY_NV = 1;
	Resources.maxMeshWorkGroupSizeZ_NV = 1;
	Resources.maxTaskWorkGroupSizeX_NV = 32;
	Resources.maxTaskWorkGroupSizeY_NV = 1;
	Resources.maxTaskWorkGroupSizeZ_NV = 1;
	Resources.maxMeshViewCountNV = 4;
	Resources.limits.nonInductiveForLoops = 1;
	Resources.limits.whileLoops = 1;
	Resources.limits.doWhileLoops = 1;
	Resources.limits.generalUniformIndexing = 1;
	Resources.limits.generalAttributeMatrixVectorIndexing = 1;
	Resources.limits.generalVaryingIndexing = 1;
	Resources.limits.generalSamplerIndexing = 1;
	Resources.limits.generalVariableIndexing = 1;
	Resources.limits.generalConstantMatrixVectorIndexing = 1;
	return Resources;
}


std::vector<unsigned int> ShaderCompilerToSPRIV(const std::string& sourceCode, glslang_stage_t shaderStage)
{

	glslang_input_t input = {};
	input.language = GLSLANG_SOURCE_GLSL;
	input.stage = shaderStage;
	input.client = GLSLANG_CLIENT_VULKAN;
	input.client_version = GLSLANG_TARGET_VULKAN_1_1;
	input.target_language = GLSLANG_TARGET_SPV;
	input.target_language_version = GLSLANG_TARGET_SPV_1_3;
	input.code = sourceCode.c_str();
	input.default_version = 450;
	input.default_profile = GLSLANG_NO_PROFILE;
	input.force_default_version_and_profile = true;
	input.forward_compatible = false;
	auto messages = GLSLANG_MSG_DEBUG_INFO_BIT | GLSLANG_MSG_DEFAULT_BIT;
	input.messages = static_cast<glslang_messages_t>(messages);
	
	input.resource = (glslang_resource_t*)(&InitResources());
	
	glslang_shader_t* shader = glslang_shader_create(&input);

	if (!glslang_shader_preprocess(shader, &input))
	{
		auto infoLog = glslang_shader_get_info_log(shader);
		auto debugInfoLog = glslang_shader_get_info_debug_log(shader);
		std::cout << __FUNCTION__": Preprocessing error! \n" << infoLog << "\n" << debugInfoLog << std::endl;
	}

	if (!glslang_shader_parse(shader, &input))
	{
		auto infoLog = glslang_shader_get_info_log(shader);
		auto debugInfoLog = glslang_shader_get_info_debug_log(shader);
		std::cout << __FUNCTION__": Compiling error! \n" << infoLog << "\n" << debugInfoLog << std::endl;
	}

	glslang_program_t* program = glslang_program_create();
	glslang_program_add_shader(program, shader);

	if (!glslang_program_link(program, 
		GLSLANG_MSG_DEFAULT_BIT | 
		GLSLANG_MSG_SPV_RULES_BIT | 
		GLSLANG_MSG_VULKAN_RULES_BIT))
	{
		auto infoLog = glslang_shader_get_info_log(shader);
		auto debugInfoLog = glslang_shader_get_info_debug_log(shader);
		std::cout << __FUNCTION__": Linking error! \n" << infoLog << "\n" << debugInfoLog << std::endl;
	}

	glslang_program_SPIRV_generate(program, input.stage);

	if (glslang_program_SPIRV_get_messages(program))
	{
		printf("%s", glslang_program_SPIRV_get_messages(program));
	}

	std::vector<unsigned int> SPIRV;
	auto code = glslang_program_SPIRV_get_ptr(program);
	for (size_t i = 0; i < glslang_program_SPIRV_get_size(program); i++)
	{
		SPIRV.push_back(code[i]);
	}
	
	glslang_shader_delete(shader);
	return SPIRV;
}


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
	vertexShaderSPIRV = ShaderCompilerToSPRIV(vertexShader, GLSLANG_STAGE_VERTEX);
	createModuleInfo.codeSize = vertexShaderSPIRV.size() * sizeof(unsigned int);
	createModuleInfo.pCode = vertexShaderSPIRV.data();
	if (vkCreateShaderModule(SmoothieCore::getDevice(), &createModuleInfo, nullptr, &vertexShaderModule) != VK_SUCCESS)
	{
		std::cout << __FUNCTION__": Failed to create vertex shader module in file: " << file << std::endl;
	}
	pipelineShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipelineShaderCreateInfo.module = vertexShaderModule;
	allShaderStages.push_back(pipelineShaderCreateInfo);


	//Fragment shader
	fragmentShaderSPIRV = ShaderCompilerToSPRIV(fragmentShader, GLSLANG_STAGE_FRAGMENT);
	createModuleInfo.codeSize = fragmentShaderSPIRV.size() * sizeof(unsigned int);
	createModuleInfo.pCode = fragmentShaderSPIRV.data();
	if (vkCreateShaderModule(SmoothieCore::getDevice(), &createModuleInfo, nullptr, &fragmentShaderModule) != VK_SUCCESS)
	{
		std::cout << __FUNCTION__": Failed to create fragment shader module in file: " << file << std::endl;
	}
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
