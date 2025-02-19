#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"
#include <iostream>
#include <vulkan/vulkan.h>
#include "SwapChain.h"
#include "DeferredPipeline.h"
#include "VMA.h"
#include "Renderer.h"
#include "UniformBuffers.h"
#include "PostProcessing.h"
#include "Shader.h"
#include "Effects/BRDF.h"
#include "Core/RenderPass.h"
#include "Core/CameraDescriptor.h"
#include "Core/Multithreading.h"

bool SmoothieCore::isEngineReady = false;

unsigned int SmoothieCore::SCR_WIDTH = 1280;
unsigned int SmoothieCore::SCR_HEIGHT = 720;
unsigned int TIME = 1;

Scene* SmoothieCore::scene = nullptr;
constexpr float apsect_ratio = static_cast<float>(1280.0f / 720.0f);
const Smoothie::Camera defaultCamera = Smoothie::Camera(
	{ -0.5, 3.0f, 18.0f }, 
	{ 0.0f, 0.0f, -1.0f },
	{ 0.0f, 1.0f, 0.0f },
	45.0f, apsect_ratio, 0.1f, 100.0f);

void SmoothieCore::initEngine(unsigned int GPUIndex, VkSurfaceKHR surface, unsigned int windowWidth, unsigned int windowHeight)
{
	SCR_WIDTH = windowWidth, SCR_HEIGHT = windowHeight;

	MultithreadSubmissions::getRenderingThreadID();
	initVulkan(GPUIndex, surface);

	SwapChain::create(windowWidth, windowHeight);
	VMA::createAllocator();

	CameraDescriptor::create();
	CameraDescriptor::update(defaultCamera.getUniformBufferData());

	auto buffer2 = &StandardUnifromBuffers::buffer2;
	buffer2->addDataInt(&SCR_WIDTH);
	buffer2->addDataInt(&SCR_HEIGHT);
	buffer2->addDataInt(&TIME);
	buffer2->createBufferFromData();
	
	Samplers::create();
	PostProcessingShaders::create();
	PostProcessing::create();
	BRDF::create();

	gBufferPass::create();
	HDRPass::create();
	PBRPass::create();
	BloomPass::create();

	Renderer::createRenderer();
	
	isEngineReady = true;
}

void SmoothieCore::finalize()
{
	removeScene();
	Renderer::destroyRenderer();
	BloomPass::destroy();
	PBRPass::destroy();
	HDRPass::destroy();
	gBufferPass::destroy();

	BRDF::destroy();
	PostProcessing::destroy();
	PostProcessingShaders::destroy();
	Samplers::destroy();

	StandardUnifromBuffers::buffer2.destroy();
	CameraDescriptor::destroy();

	VMA::freeAllocator();
	SwapChain::destroy();
	destroyEngine();
}

void SmoothieCore::loadScene(const std::string& scene_file)
{
	if (isEngineReady == false) 
	{
		std::cout << __FUNCTION__": Can't load scene because engine is not initilized!" << std::endl;
	}
	else if (scene != nullptr)
	{
		std::cout << __FUNCTION__": Scene is already loaded!" << std::endl;
	}
	else
	{
		std::cout << __FUNCTION__": Loading scene: " << scene_file << std::endl;
		scene = new Scene(scene_file);
	}
}

void SmoothieCore::removeScene()
{
	if (scene != nullptr)
	{
		scene->destroy();
		delete scene;
		scene = nullptr;
	}
}

void SmoothieCore::updateCameraData(const Smoothie::Camera& camera)
{
	CameraDescriptor::update(camera.getUniformBufferData());
}

void SmoothieCore::updateRenderingResolution(unsigned int windowWidth, unsigned int windowHeight)
{
	vkDeviceWaitIdle(device);
	if (windowWidth == 0 && windowHeight == 0) 
	{ 
		isEngineReady = false; 
		return; 
	}
	else
	{
		SCR_WIDTH = windowWidth, SCR_HEIGHT = windowHeight;
		isEngineReady = true;
		SwapChain::update(windowWidth, windowHeight);
		if (scene != nullptr) 
		{
			scene->update(windowWidth, windowHeight);
		}
	}
}

void SmoothieCore::setViewport(VkCommandBuffer commandBuffer)
{
	//Dynamic properties of a pipeline
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(SCR_WIDTH);
	viewport.height = static_cast<float>(SCR_HEIGHT);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void SmoothieCore::setScissor(VkCommandBuffer commandBuffer)
{
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent.width = SCR_WIDTH;
	scissor.extent.height = SCR_HEIGHT;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void SmoothieCore::draw()
{
	if (scene == nullptr) return;
	TIME++;
	Renderer::draw();
}
