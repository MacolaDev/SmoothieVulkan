#pragma once
#include "Common.h"

namespace Smoothie
{
    struct Geometry_Data_Type
    {
        enum Base_Type
        {
            Unknown,
            Int_8,
            Int_16,
            Int_32,
            Int_64,
            UInt_8,
            UInt_16,
            UInt_32,
            UInt_64,
            Float_32
        };

        Base_Type base_type = Unknown;
        unsigned int count = 0;
    };

    struct Geometry_Attribute
    {
        std::uint32_t offset = 0;
        std::uint32_t size = 0;
        Geometry_Data_Type type;
        Geometry_Data_Type original_type;
        std::vector<std::uint8_t> min_values;
        std::vector<std::uint8_t> max_values;
    };

    struct Geometry_Mesh_LOD
    {
        std::uint32_t vertex_offset = 0;
        std::uint32_t vertex_count = 0;
        std::uint32_t index_offset = 0;
        std::uint32_t index_count = 0;
    };

    struct Geometry_Mesh
    {
        VkIndexType index_type = VK_INDEX_TYPE_MAX_ENUM;
        VkPrimitiveTopology index_topology = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
        std::vector<Geometry_Attribute> vertex_buffer_attributes;
        std::vector<Geometry_Mesh_LOD> LODs;

        std::vector<std::uint8_t> vertex_data_raw;
        std::vector<std::uint8_t> index_data_raw;

        VkBuffer vertex_buffer = nullptr;
        VmaAllocation vertex_buffer_allocation = nullptr;
        VkBuffer index_buffer = nullptr;
        VmaAllocation index_buffer_allocation = nullptr;
    };

    enum GeometryFile_CreateFlags: int
    {
        GeometryFile_Flags_None = 0,
        GeometryFile_Flags_Dont_Create_Buffers = 1 << 0,
        GeometryFile_Flags_Hold_raw_data = 1 << 1
    };

    class GeometryFile
    {
        static constexpr char s_Magic[4] = {7, 8, 26, 0};
    public:
        using MeshMap = std::unordered_map<std::string, Geometry_Mesh>;
        const MeshMap& getMeshes() const {return m_Meshes;}

        int create(const std::string& filepath, GeometryFile_CreateFlags flags = GeometryFile_Flags_None);
        void destroy();

        static constexpr const char* get_Magic() {return s_Magic;}

    protected:

        virtual int create_buffers(MeshMap& meshes, GeometryFile_CreateFlags flags);

        std::unordered_map<std::string, Geometry_Mesh> m_Meshes;
    };
}