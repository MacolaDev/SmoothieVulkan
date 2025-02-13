#pragma once
#include <vulkan/vulkan.h>
#include "Core/Image.h"
#include <array>

struct MipChainImage
{
	Image mipImage;
	unsigned int width = 1, height = 1;
	VkFramebuffer framebuffer = nullptr;

	//downsampling data
	VkDescriptorPool dwDescriptorPool = nullptr;
	VkDescriptorSetLayout dwDescriptorSetLayout = nullptr;
	VkDescriptorSet dwDescriptorSet = nullptr;
	VkPipelineLayout dwPipelineLayout = nullptr;
	VkPipeline dwPipeline = nullptr;

	//Upsamling
	VkDescriptorPool upDescriptorPool = nullptr;
	VkDescriptorSetLayout upDescriptorSetLayout = nullptr;
	VkDescriptorSet upDescriptorSet = nullptr;
	VkPipelineLayout upPipelineLayout = nullptr;
	VkPipeline upPipeline = nullptr;


};

constexpr int numberOfMips = 5;
constexpr float filterRadius = 0.0025f;

class Bloom
{

	static VkDescriptorPool higlightDescriptorPool;
	static VkDescriptorSetLayout higlightDescriptorSetLayout;
	static VkDescriptorSet higlightDescriptorSet;

	static VkPipeline higlightPipeline;
	static VkPipelineLayout higlightPipelineLayout;

	static std::array<MipChainImage, numberOfMips> mipChainImages;

	static VkPipeline downsamplingPipeline;
	static VkPipelineLayout downsamplingPipelineLayout;

public:
	static void create(unsigned int width, unsigned int height, const Image& HDRIImage);
	static void bindAndDraw(VkCommandBuffer commandBuffer);
	static void destroy();
	static Image getBloomImage();
};

