#pragma once
#include "Common.h"
#include "Core/Shader.h"

#include "../Base.h"

namespace Smoothie{
namespace DeferredRendering
{
    class Sky: public Pipeline_Base
    {
    protected:
        ShaderFile m_ShaderFile;

        VkFormat m_Format_Sky = VK_FORMAT_R32G32B32A32_SFLOAT;
        VkExtent3D m_Extent_Sky = { 512, 512, 1 };
        VkImage m_Image_Sky = nullptr;
        VkImageView m_ImageView_Sky = nullptr;
        VmaAllocation m_Allocation_Image_Sky = nullptr;
        virtual int create_image_sky();
        virtual void destroy_image_sky();

        std::string m_DescriptorSet_Name = "g_Sky";
        VkDescriptorSet m_DescriptorSet = nullptr;
        VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
        VkDescriptorPool m_DescriptorPool = nullptr;
        virtual int create_descriptor_set();
        virtual void destroy_descriptor_set();

        std::string m_Pipeline_Skybox_VertexShaderEntryName = "vertex_skybox";
        std::string m_Pipeline_Skybox_FragmentShaderEntryName = "fragment_skybox";
        VkPipeline m_Pipeline_Skybox = nullptr;
        VkPipelineLayout m_PipelineLayout_Skybox = nullptr;
        virtual int create_pipeline_skybox();
        virtual void destroy_pipeline_skybox();

    public:

        inline VkImage get_Image_Sky() const { return m_Image_Sky; }
        inline VkImageView get_Image_SkyView() const { return m_ImageView_Sky; }

        int create(const std::string& shader) override;
        int update() override;
        void record_data(const Pipeline_Data &drawData) const override;
        void destroy() override;

    };
}

}

