#pragma once
#include <vector>
#include <memory>

#include "Core/Model.h"
#include "Core/Mesh.h"

#include "Math/SmoothieMath.h"

namespace Smoothie 
{
	namespace DeferredRendering 
	{
		class BasePipeline;

		//Standard models that can be drawn inside the deferred rendering pipeline.
		class Deferred_Model : public Model_Base
		{
		protected:
			std::shared_ptr<BasePipeline> m_pPipeline = nullptr;
			std::vector<SmoothieMath::Matrix4x4> m_ModelMatrices;
			SmoothieMath::Vector3 m_BBMin;
			SmoothieMath::Vector3 m_BBMax;
			ModelFile m_modelFile;
		public:

			inline void set_pipeline(const std::shared_ptr<BasePipeline>& pPipeline) {m_pPipeline = pPipeline;}

			void add_model_matrix(const SmoothieMath::Matrix4x4& matrix);
			inline const SmoothieMath::Vector3& getBBMin() const {return m_BBMin;}
			inline const SmoothieMath::Vector3& getBBMax() const {return m_BBMax;}
			inline const std::vector<SmoothieMath::Matrix4x4>& getModelMatrices() const {return m_ModelMatrices;}

			inline void setModelFile(const std::string& file) {m_modelFile.set_model_file(file);}
			inline const std::string& getModelFile() const {return m_modelFile.getModelFile();}

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

		};
		
		//Most standard model used for static drawing on the scene.
		class Standard_Model : public Deferred_Model
		{
		protected:
			SmoothieMath::Matrix4x4 modelMatrix;
			VkDescriptorPool descriptorPool = nullptr;
			VkDescriptorSet descriptorSet = nullptr;

			VkBuffer m_ModelUniformBuffer = nullptr;
			VmaAllocation m_ModelUniformBufferAllocation = nullptr;

			VkBuffer m_vertexBuffer = nullptr;
			VkBuffer m_indexBuffer = nullptr;
			VkIndexType m_indexType = VK_INDEX_TYPE_UINT32;
			unsigned int m_numberOfIndices = 0;

			std::vector<std::string> m_Textures;
			unsigned int doubleSided = 0;

		public:
			int create() override;
			int update() override;

			void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet drawingClassDescriptor, unsigned int FrameID) const override;
			void drawShadow(const SmoothieMath::Matrix4x4 lightProjViewMat, VkPipelineLayout pipelineLayout, VkCommandBuffer commandBuffer, unsigned int FrameID) const override;
			void destroy() override;

		};
	}

}

