#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"
#include <iostream>
#include <vulkan/vulkan.h>
#include "SwapChain.h"
#include "DeferredPipeline.h"
#include "VMA.h"
#include <glslang/Public/ShaderLang.h>
#include "Renderer.h"
#include "UniformBuffers.h"
#include "Image.h"

bool SmoothieCore::isEngineReady = false;
Smoothie::Camera* SmoothieCore::camera = nullptr;

int SmoothieCore::SCR_WIDTH = 1280;
int SmoothieCore::SCR_HEIGHT = 720;
int TIME = 1;

void SmoothieCore::initEngine(unsigned int GPUIndex, VkSurfaceKHR surface, unsigned int windowWidth, unsigned int windowHeight, Smoothie::Camera* camera)
{
	initVulkan(GPUIndex, surface);
	SwapChain::create(windowWidth, windowHeight);
	VMA::createAllocator();
	glslang::InitializeProcess();

	SmoothieCore::camera = camera;

	auto buffer1 = &StandardUnifromBuffers::buffer1;
	auto buffer2 = &StandardUnifromBuffers::buffer2;
	buffer1->addDataMatrix4x4(&camera->projectionMatrix);
	buffer1->addDataMatrix4x4(&camera->cameraMatrix);
	buffer1->addDataVector3(&camera->cameraPos);
	buffer1->addDataMatrix4x4(&camera->projectionViewMatrix);
	buffer1->createBufferFromData();
	
	buffer2->addDataInt(&SCR_WIDTH);
	buffer2->addDataInt(&SCR_HEIGHT);
	buffer2->addDataInt(&TIME);
	buffer2->createBufferFromData();
	
	Samplers::create();
	DeferredPipeline::create(windowWidth, windowHeight);

	Renderer::createRenderer();
}

void SmoothieCore::finalize()
{
	Renderer::destroyRenderer();

	DeferredPipeline::destroy();
	Samplers::destroy();

	StandardUnifromBuffers::buffer2.destroy();
	StandardUnifromBuffers::buffer1.destroy();


	glslang::FinalizeProcess();
	VMA::freeAllocator();
}

void SmoothieCore::draw()
{
	TIME++;
	Renderer::draw();
}
