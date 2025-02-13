#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include "ResourceManager/ShaderFile.h"
#include <vector>
#include "ResourceManager/ResourceHandler.h"

class Shader: private ShaderFile
{
	VkShaderModule vertexShaderModule = nullptr;
	VkShaderModule fragmentShaderModule = nullptr;

	std::vector<VkPipelineShaderStageCreateInfo> allShaderStages;
public:
	Shader(const std::string& file);
	Shader() = default;
	void destroy();
	std::vector<VkPipelineShaderStageCreateInfo> getPipelineShaderStages() const;
};

enum class ShaderModelRenderPass
{
	UNDEFINED = 0,
	gBuffer = 1, 
	HDRPass = 2
};

struct ModelShader: private ResourceHandler<ModelShader>
{
	VkShaderModule vertexShaderModule = nullptr;
	VkShaderModule fragmentShaderModule = nullptr;

	void create(const std::string& shaderFile);
	void destroy();

	ModelShader() = default;
	ModelShader(const std::string& shaderFile);

	ShaderModelRenderPass getRenderPass() const;

private:
	std::string filepath;
};


//Shaders that post processing and screen-space effects (like lighting calculations in PBR rendering) will be used.
//This shaders are independed from shaders that models use for rendering objects in the world.
struct PostProcessingShaders 
{
	static VkShaderModule basicVertexShader; //draws a quad on a screen
	static VkShaderModule PBRShader;
	static VkShaderModule BRDF; 

	static VkShaderModule cubemapVertex; //Used for hdri cubemap, irradiance map, prefilter
	static VkShaderModule HDRToCubemap;
	static VkShaderModule Irradiance;
	static VkShaderModule Prefilter;

	static VkShaderModule SkyboxVertex;
	static VkShaderModule SkyboxFragment;

	static VkShaderModule higlights;
	static VkShaderModule downsampling;
	static VkShaderModule upsampling;

	static VkShaderModule DeferredPipelineFinal;

	static void create();
	static void destroy();
};
