#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include "ResourceManager/ShaderFile.h"
#include <vector>

class Shader: private ShaderFile
{
	std::vector<unsigned int> vertexShaderSPIRV;
	VkShaderModule vertexShaderModule = nullptr;

	std::vector<unsigned int> fragmentShaderSPIRV;
	VkShaderModule fragmentShaderModule = nullptr;

	std::vector<VkPipelineShaderStageCreateInfo> allShaderStages;
public:
	Shader(const std::string& file);
	Shader() = default;
	void destroy();

	std::vector<VkPipelineShaderStageCreateInfo> getPipelineShaderStages() const;
};

