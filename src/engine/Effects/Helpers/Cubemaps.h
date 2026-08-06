#pragma once
#include "Common.h"
#include "Core/Shader.h"

namespace Smoothie
{namespace DeferredRendering
{
    class Helper_HDR_CubeMaps
    {
    protected:
        ShaderFile m_ShaderFile;

        VkFormat m_Format = VK_FORMAT_R32G32B32A32_SFLOAT;

        unsigned int m_Size_EnvironmentMap = 512;
        VkImage m_Image_EnvironmentMap = nullptr;
        VkImageView m_ImagView_EnvironmentMap = nullptr;
        VmaAllocation m_Allocation_EnvironmentMap = nullptr;

        unsigned int m_Size_Irradiance = 32;
        VkImage m_Image_Irradiance = nullptr;
        VkImageView m_ImagView_Irradiance = nullptr;
        VmaAllocation m_Allocation_Irradiance = nullptr;
        std::array<VkImageView, 6> m_Irradiance_Target_ImageViews = {};

        unsigned int m_Size_Prefilter = 128;
        unsigned int m_MipCount_Prefilter = 5;
        VkImage m_Image_Prefilter = nullptr;
        VkImageView m_ImagView_Prefilter = nullptr;
        VmaAllocation m_Allocation_Prefilter = nullptr;
        std::array<std::vector<VkImageView>, 6> m_Prefilter_Target_ImageViews = {};

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
        std::string m_StageEntryName_Irradiance_Map = "irradiance_map";
        std::string m_StageEntryName_Prefilter_Map = "prefilter_map";

        VkPipelineLayout m_PipelineLayout = nullptr;

        VkPipeline m_Pipeline_HDR_To_CubeMap = nullptr;
        VkPipeline m_Pipeline_Irradiance_Map = nullptr;
        VkPipeline m_Pipeline_Prefilter_Map = nullptr;
        virtual int create_pipelines();
        virtual void destroy_pipelines();

    public:
        virtual int create(const std::string& shader);
        virtual void destroy();

        virtual int create_cubmap_from_hdr(const std::string& filepath);
        virtual int copy_env_map_to_skybox_cubemap(VkImage skybox_image, VkExtent3D sky_extend, VkFormat sky_format);
        virtual int calculate_irradiance_prefilter_maps();

        virtual ~Helper_HDR_CubeMaps() = default;
    };
}
}