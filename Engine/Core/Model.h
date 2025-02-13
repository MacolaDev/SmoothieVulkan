#pragma once
#include "Mesh.h"
#include "Shader.h"
#include <string>
#include <vulkan/vulkan.h>
#include <Core/ShaderProperty.h>
#include <Math/SmoothieMath.h>


class Scene;
class DeferredPipeline;
namespace Smoothie 
{
	class LoadedModels;
	class ShaderProperty;
	class Model
	{
	public:

		Model(const std::string& modelFile, SmoothieMath::Matrix4x4 modelMatrix = SmoothieMath::Matrix4x4());

		void addToRendering() const;
		void removeFromRendering() const;

		Model() = default;

		unsigned int getModelID() const;
		
	private:
		
		void destroy();
		void bindAndDraw(VkCommandBuffer commandBuffer) const;
		
		friend class ShaderProperty;
		friend class DeferredPipeline;
		friend class LoadedModels;
		friend class Scene;

		unsigned int ModelID = 0;
		Mesh mesh;

		ShaderModelRenderPass modelRenderPass = ShaderModelRenderPass::UNDEFINED;
		ModelShader shader;

		ShaderProperty properties;
		std::vector<ModelMatrices> modelMatrices;
		VkPipeline modelPipeline = nullptr;
		VkPipelineLayout pipelineLayout = nullptr;

	};
}
