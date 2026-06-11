#pragma once
#include "Core/Common.h"
#include "Core/Shader.h"

#include "../Base.h"

namespace Smoothie{
namespace DeferredRendering
{
    class Sky: public Pipeline_Base
    {
    protected:
        ShaderFile m_ShaderFile;

        VkDescriptorSet m_DescriptorSet = nullptr;
        VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
        VkDescriptorPool m_DescriptorPool = nullptr;

        std::string m_Pipeline_Skybox_VertexShaderEntryName = "vertex_skybox";
        std::string m_Pipeline_Skybox_FragmentShaderEntryName = "fragment_skybox";
        VkPipeline m_Pipeline_Skybox = nullptr;
        VkPipelineLayout m_PipelineLayout_Skybox = nullptr;
        virtual int create_pipeline_skybox();
        virtual void destroy_pipeline_skybox();

    public:
        int create(const std::string& shader) override;
        int update() override;
        void record_data(const Pipeline_Data &drawData) const override;
        void destroy() override;

    };
}

}

