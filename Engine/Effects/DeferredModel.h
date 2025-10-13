#pragma once
#include <memory>

#include "Core/Model.h"
#include "Core/Mesh.h"

#include "Effects/DeferredBuffer.h"

#include "Math/SmoothieMath.h"

namespace Smoothie 
{
	namespace DeferredRendering 
	{
	
		//Standard models that can be drawn inside the deferred rendering
		class DeferredModel : public Model_Base
		{
		protected:

		public:
			virtual void draw(
				VkCommandBuffer commandBuffer,
				VkPipelineLayout pipelineLayout,
				VkDescriptorSet drawingClassDescriptor,
				unsigned int ImageIndex) const = 0;

			virtual void drawShadow(
				const SmoothieMath::Matrix4x4 lightProjViewMat,
				VkPipelineLayout pipelineLayout,
				VkCommandBuffer commandBuffer,
				unsigned int FrameID) const = 0;
			DeferredModel() = default;
		};
		
		//Most standard model used for static drawing on the scene.
		class StandardModel : public DeferredModel
		{
		protected:
			SmoothieMath::Matrix4x4 modelMatrix;
			VkDescriptorPool descriptorPool = nullptr;
			VkDescriptorSet descriptorSet = nullptr;

			VkBuffer m_ModelUniformBuffer = nullptr;
			VmaAllocation m_ModelUniformBuffferAllocation = nullptr;

			VkBuffer m_vertexBuffer = nullptr;
			VkBuffer m_indexBuffer = nullptr;
			VkIndexType m_indexType = VK_INDEX_TYPE_UINT32;
			unsigned int m_numberOfIndecies = 0;

			std::vector<std::string> m_Textures;
			unsigned int doubleSided = 0;
			ModelFile modelFile;

		public:
			int create() override;

			void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet drawingClassDescriptor, unsigned int FrameID) const override;
			void drawShadow(const SmoothieMath::Matrix4x4 lightProjViewMat, VkPipelineLayout pipelineLayout, VkCommandBuffer commandBuffer, unsigned int FrameID) const override;
			void destroy() override;

			inline void setModelMatrix(const SmoothieMath::Matrix4x4& modelMatrix) { this->modelMatrix = modelMatrix; }
			inline SmoothieMath::Matrix4x4 getModelMatrix() const { return modelMatrix; }
			inline void setModelFile(const ModelFile& file) { this->modelFile = file; }
			inline const ModelFile& getModelFile() const { return modelFile; }

			StandardModel() = default;
		};
	}

}

