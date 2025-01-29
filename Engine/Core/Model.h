#pragma once
#include "Mesh.h"
#include "Shader.h"
#include <string>
#include <vulkan/vulkan.h>
#include <Core/ShaderProperty.h>
#include <Math/SmoothieMath.h>

class Scene;
namespace Smoothie 
{
	class ShaderProperty;
	class Model
	{
		std::string modelFile;
		std::string meshFile;
		
		Mesh modelMesh;
		Shader modelShader;
		friend class Scene;
		friend class ShaderProperty;
		ShaderProperty properties;

		std::vector<ModelMatrices> modelMatrices;

		VkPipeline modelPipeline = nullptr;
		VkPipelineLayout modelPipelineLayout;
		
	public:
		Model() = default;
		Model(const std::string& modelFile, SmoothieMath::Matrix4x4 modelMatrix = SmoothieMath::Matrix4x4());

		void destroy();
		void bindAndDraw(VkCommandBuffer commandBuffer) const;

		std::string getModelFile() const; 
		std::string getMeshFile() const;

	};
}
