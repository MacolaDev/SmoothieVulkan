#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

#include "Core/Pipeline.h"
#include "Core/Shader.h"
#include "Core/Descriptor.h"

#include "Effects/Deferred_Model.h"
#include "Math/SmoothieMath.h"


namespace Smoothie 
{
	namespace DeferredRendering 
	{

		struct RecordCommandsData
		{
			VkCommandBuffer commandBuffer_gBuffer = nullptr;
			VkCommandBuffer commandBuffer_HDR = nullptr;
			VkCommandBuffer commandBuffer_Compute = nullptr;
			VkCommandBuffer commandBuffer_Shadow = nullptr;

			VkDescriptorSet descriptorSet_DrawClass = nullptr;
			unsigned int imageIndex = 0;
		};


		//Base pipeline for all deferred rendering.
		class BasePipeline : public Pipeline_Base
		{
		protected:
			VkPipeline m_Pipeline = nullptr;
			VkPipelineLayout m_PipelineLayout = nullptr;

			VkPipeline m_PipelineShadow = nullptr;
			VkPipelineLayout m_PipelineShadowLayout = nullptr;
			mutable unsigned int m_UseCount = 0;
			ShaderFile m_ShaderFile;


			int m_tmpRenderPass = 0;
		public:

			inline const ShaderFile& getShaderFile() const { return m_ShaderFile; }
			inline void setShaderFile(const ShaderFile& shaderFile) { m_ShaderFile = shaderFile; }

			virtual void record_commands(const RecordCommandsData& buffers) const = 0;

			virtual void bindAndDraw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, unsigned int ImageIndex) const = 0;
			virtual void bindAndDrawShadow(const SmoothieMath::Matrix4x4& lightProjectionViewMat, VkCommandBuffer commandBuffer, unsigned int ImageIndex) const = 0;

			virtual int add_to_rendering(std::shared_ptr<Deferred_Model> model) const = 0;
			virtual void remove_from_rendering(std::shared_ptr<Deferred_Model> model) const = 0;

			int update() override;
		};

		//***********************************************************************************
		//Standard gBuffer or HDR static model. I use it in world rendering for static models 
		//***********************************************************************************
		class StaticPipeline : public BasePipeline
		{

			mutable std::vector<std::shared_ptr<Deferred_Model>> m_Models;
			VkDescriptorSetLayout m_PipelineDescriptorSetLayout = nullptr;
		public:
			int create() override;
			void bindAndDraw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, unsigned int ImageIndex) const override final;
			void bindAndDrawShadow(const SmoothieMath::Matrix4x4& lightProjectionViewMat, VkCommandBuffer commandBuffer, unsigned int ImageIndex) const override final;

			void record_commands(const RecordCommandsData& buffers) const override;

			int add_to_rendering(std::shared_ptr<Deferred_Model> model) const override final;
			void remove_from_rendering(std::shared_ptr<Deferred_Model> model) const override final;

			inline VkDescriptorSetLayout getPipelineDescriptorSetLayout() const { return m_PipelineDescriptorSetLayout; }


			void destroy() override final;
		};

	}
}
