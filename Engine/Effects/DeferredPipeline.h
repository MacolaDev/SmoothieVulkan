#pragma once
#include <vulkan/vulkan.h>
#include <list>
#include <memory>
#include <vector>

#include "Effects/DeferredModel.h"
#include "Core/RenderPass.h"
#include "Core/Pipeline.h"
#include "Core/Shader.h"

#include "vk_mem_alloc.h"
#include "Math/SmoothieMath.h"


namespace Smoothie 
{
	namespace DeferredRendering 
	{
		//************************************************
		//Model pipelines that can be drawn in both HDR and gBuffer pass.
		//************************************************
		class BasePipeline : public Pipeline_Base
		{
		protected:
			VkPipeline shadow_pipeline = nullptr;
			VkPipelineLayout shadow_pipelineLayout = nullptr;
			VkDescriptorSetLayout drawingClassDescriptorSetLayout = nullptr;

		public:
			virtual void bindAndDraw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, unsigned int ImageIndex) const = 0;
			virtual void bindAndDrawShadow(const SmoothieMath::Matrix4x4& lightProjectionViewMat, VkCommandBuffer commandBuffer, unsigned int ImageIndex) const = 0;

			virtual void add_to_rendering(std::shared_ptr<DeferredModel> model) const = 0;
			virtual void remove_from_rendering(std::shared_ptr<DeferredModel> model) const = 0;
			inline void setDrawingClassDescriptrSetLayout(VkDescriptorSetLayout layout) { this->drawingClassDescriptorSetLayout = layout; }
		};

		//***********************************************************************************
		//Standard gBuffer or HDR static model. I use it in world rendering for static models 
		//***********************************************************************************
		class StaticPipeline : public BasePipeline
		{

			mutable std::vector<std::shared_ptr<DeferredModel>> m_Models;
			DescriptorSetData m_DescriptorData;
			VkDescriptorSetLayout m_PipelineDescriptorSetLayout = nullptr;
		public:
			int create(const ShaderFile& shaderFile) override final;
			void bindAndDraw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, unsigned int ImageIndex) const override final;
			void bindAndDrawShadow(const SmoothieMath::Matrix4x4& lightProjectionViewMat, VkCommandBuffer commandBuffer, unsigned int ImageIndex) const override final;

			void add_to_rendering(std::shared_ptr<DeferredModel> model) const override final;
			void remove_from_rendering(std::shared_ptr<DeferredModel> model) const override final;

			inline const DescriptorSetData& getPipelineDescriptorsData() const { return m_DescriptorData; }
			inline VkDescriptorSetLayout getPipelineDescriptorSetLayout() const { return m_PipelineDescriptorSetLayout; }


			void destroy() override final;
		};

	}
}
