#pragma once
#include "Base.h"

namespace Smoothie 
{
	namespace DeferredRendering
	{
	    //Standard models that can be drawn inside the deferred rendering pipeline.
	    class Model_Standard : public Model_Base
	    {
	    protected:
	        // std::vector<SmoothieMath::Matrix4x4> m_ModelMatrices;
	        // SmoothieMath::Matrix4x4 modelMatrix;

	        VkDescriptorPool m_DescriptorPool = nullptr;
	        VkDescriptorSet m_DescriptorSet = nullptr;

	        VkBuffer m_ModelUniformBuffer = nullptr;
	        VmaAllocation m_ModelUniformBufferAllocation = nullptr;

	        VkBuffer m_vertexBuffer = nullptr;
	        VkBuffer m_indexBuffer = nullptr;
	        VkIndexType m_indexType = VK_INDEX_TYPE_UINT32;
	        unsigned int m_numberOfIndices = 0;
	        unsigned int doubleSided = 0;

	        std::string m_Filepath;
	        int create() override;
	    public:

	        // void add_model_matrix(const SmoothieMath::Matrix4x4& matrix);
	        // inline const std::vector<SmoothieMath::Matrix4x4>& getModelMatrices() const {return m_ModelMatrices;}


	        inline int create(const std::string& file) {m_Filepath = file;return create();}
	        int update() override;
	        void destroy() override;

	        void draw(
                VkCommandBuffer commandBuffer,
                VkPipelineLayout pipelineLayout,
                VkDescriptorSet drawingClassDescriptor,
                unsigned int ImageIndex) const;
	    };
	}
}

