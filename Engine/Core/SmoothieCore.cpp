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

bool SmoothieCore::isEngineReady = false;

int SmoothieCore::SCR_WIDTH = 1280;
int SmoothieCore::SCR_HEIGHT = 720;
int TIME = 1;

Scene* SmoothieCore::scene = nullptr;
constexpr float apsect_ratio = static_cast<float>(1280.0f / 720.0f);
const Smoothie::Camera defaultCamera = Smoothie::Camera(
	{ -0.5, 3.0f, 18.0f }, 
	{ 0.0f, 0.0f, -1.0f },
	{ 0.0f, 1.0f, 0.0f },
	45.0f, apsect_ratio, 0.1f, 100.0f);

void SmoothieCore::initEngine(unsigned int GPUIndex, VkSurfaceKHR surface, unsigned int windowWidth, unsigned int windowHeight)
{
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

void SmoothieCore::draw()
{
	if (scene == nullptr) return;
	TIME++;
	Renderer::draw();
}
