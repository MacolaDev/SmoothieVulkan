#pragma once

#include "Core/Common.h"
#include "Core/Shader.h"

namespace Smoothie{
namespace DeferredRendering{

    class Light_Global
    {
    protected:

        ShaderFile m_ShaderFile;

        std::string m_ShaderEntryName_BRDF = "brdf_precompute";

        std::string m_DescriptorSetName_BRDF = "brdf_LUT";
        VkImage m_Image_BRDF = nullptr;
        VkImageView m_ImageView_BRDF = nullptr;
        VmaAllocation m_Allocation_BRDF = nullptr;
        VkFormat m_Format_BRDF = VK_FORMAT_R16G16_SFLOAT;
        VkExtent3D m_Extent_BRDF = { 512 , 512 , 1 };
        virtual int create_brdf();
        virtual void destroy_brdf();

        std::string m_DescriptorSetName_Irradiance = "irradiance_map";
        VkImage m_Image_Irradiance = nullptr;
        VkImageView m_ImageView_Irradiance = nullptr;
        VmaAllocation m_Allocation_Irradiance = nullptr;
        VkFormat m_Format_Irradiance = VK_FORMAT_R32G32B32A32_SFLOAT;
        VkExtent3D m_Extent_Irradiance = { 32 , 32 , 1 };
        virtual int create_irradiance();
        virtual void destroy_irradiance();

        std::string m_DescriptorSetName_Prefilter = "prefilter_map";
        VkImage m_Image_Prefilter = nullptr;
        VkImageView m_ImageView_Prefilter = nullptr;
        VmaAllocation m_Allocation_Prefilter = nullptr;
        VkFormat m_Format_Prefilter = VK_FORMAT_R32G32B32A32_SFLOAT;
        VkExtent3D m_Extent_Prefilter = { 128 , 128 , 1 };
        unsigned int m_MipLevels_Prefilter = 5;
        virtual int create_prefiler();
        virtual void destroy_prefiler();

        std::string m_DescriptorSet_Name = "g_LightingGlobal";
        VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
        VkDescriptorPool m_DescriptorPool = nullptr;
        VkDescriptorSet m_DescriptorSet = nullptr;
        virtual int create_descriptors();
        virtual void destroy_descriptors();

        std::string m_ShaderEntryName_Light = "global_illumination";
        VkPipelineLayout m_PipelineLayout = nullptr;
        VkPipeline m_Pipeline = nullptr;
        virtual int create_pipeline();
        virtual void destroy_pipeline();

    public:

        int create(const std::string &shader);
        void draw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, unsigned int width, unsigned int height) const;
        void destroy();


        virtual ~Light_Global() = default;
    };

}
}
