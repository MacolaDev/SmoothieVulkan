#pragma once
#include "Core/Common.h"
#include "Core/Shader.h"

namespace Smoothie
{namespace DeferredRendering
{
    class Helper_HDR_CubeMaps
    {
    protected:
        ShaderFile m_ShaderFile;

        VkExtent3D m_Extent_EnvironmentMap = {512, 512, 1};
        VkFormat m_Format_EnvironmentMap = VK_FORMAT_R32G32B32A32_SFLOAT;
        VkImage m_Image_EnvironmentMap = nullptr;
        VkImageView m_ImagView_EnvironmentMap = nullptr;
        VmaAllocation m_Allocation_EnvironmentMap = nullptr;

        VkImage m_Image_Irradiance = nullptr;
        VkImageView m_ImagView_Irradiance = nullptr;
        VmaAllocation m_Allocation_Irradiance = nullptr;

        VkImage m_Image_Prefilter = nullptr;
        VkImageView m_ImagView_Prefilter = nullptr;
        VmaAllocation m_Allocation_Prefilter = nullptr;

        virtual int create_images();
        virtual void destroy_images();

        struct alignas(16) UniformBufferData
        {
            glm::mat4 projectView;
            float roughness;
        };

        VkBuffer m_Buffer_Uniform = nullptr;
        VmaAllocation m_Allocation_Uniform = nullptr;
        void* m_pBufferData = nullptr;
        virtual int create_buffer_uniform();
        virtual void destroy_buffer_uniform();
        virtual void copy_buffer_data_uniform(VkCommandBuffer commandBuffer, const UniformBufferData& data);

        std::string m_DescriptorSet_Name = "g_Globals";
        VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
        VkDescriptorPool m_DescriptorPool = nullptr;
        VkDescriptorSet m_DescriptorSet = nullptr;
        virtual int create_descriptor_set();
        virtual void destroy_descriptor_set();


        std::string m_StageEntryName_Vertex = "cubemaps_vertex";
        std::string m_StageEntryName_HDR_To_Cubemap = "hdr_to_cubemap";
        std::string m_StageEntryName_HDR_To_Irradiance_Map = "irradiance_map";
        std::string m_StageEntryName_HDR_To_Prefilter_Map = "prefilter_map";

        VkPipelineLayout m_PipelineLayout = nullptr;

        VkPipeline m_Pipeline_HDR_To_CubeMap = nullptr;
        VkPipeline m_Pipeline_Irradiance_Map = nullptr;
        VkPipeline m_Pipeline_Prefilter_Map = nullptr;
        virtual int create_pipelines();
        virtual void destroy_pipelines();

    public:
        virtual int create(const std::string& shader);
        virtual void destroy();



        virtual ~Helper_HDR_CubeMaps() = default;
    };
}
}