#pragma once
#include "Base.h"

namespace Smoothie 
{
	namespace DeferredRendering 
	{
		class Pipeline_Standard : public Pipeline_Base
		{

		    VkPipeline m_Pipeline_HDR = nullptr;
		    VkPipeline m_Pipeline_gBuffer = nullptr;
		    VkPipelineLayout m_PipelineLayout = nullptr;

		    VkPipeline m_PipelineShadow = nullptr;
		    VkPipelineLayout m_PipelineLayoutShadow = nullptr;


		    std::vector<std::shared_ptr<Model_Standard>> m_Models;
			VkDescriptorSetLayout m_PipelineDescriptorSetLayout = nullptr;
		public:
			int create(const std::string& shader) override;

			void record_data(const Pipeline_Data& buffers) const override;

			int add_to_rendering(const std::shared_ptr<Model_Standard>& model);
			void remove_from_rendering(const std::shared_ptr<Model_Standard>& model);

			inline VkDescriptorSetLayout getPipelineDescriptorSetLayout() const { return m_PipelineDescriptorSetLayout; }


			void destroy() override;
		};

	}
}
