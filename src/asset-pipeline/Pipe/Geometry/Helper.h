#pragma once
#include "Common.h"

namespace Smoothie::AssetPipeline::Geometry_Helpers
{
    struct VertexBuffer_Template_Argument;
    using Conversion_Function = std::function<void(std::vector<std::uint8_t>&, const VertexBuffer_Template_Argument&)>;

    void Conversion_Function_cast(std::vector<std::uint8_t>& data, const VertexBuffer_Template_Argument& vertex_template);
    void Conversion_Function_pack_uv(std::vector<std::uint8_t>& data, const VertexBuffer_Template_Argument& vertex_template);
    void Conversion_Function_pack_normals_10_10_10_2(std::vector<std::uint8_t>& data, const VertexBuffer_Template_Argument& vertex_template);

    struct VertexBuffer_Template_Argument
    {
        unsigned int location = 0;
        unsigned int offset = 0;
        unsigned int size = 0;
        std::vector<std::string> names;
        Smoothie::Geometry_Data_Type input;
        Smoothie::Geometry_Data_Type output;
        Conversion_Function conversion_function;
    };

    int get_type_size(Geometry_Data_Type::Base_Type type);

    using VertexBuffer_Template = std::vector<VertexBuffer_Template_Argument>;
    using VertexBuffer_Templates = std::unordered_map<std::string, VertexBuffer_Template>;
    int parse_vertex_buffer_templates(const std::string& input_file, VertexBuffer_Templates& templates);

    struct WorkData_VertexBuffer
    {
        VertexBuffer_Template_Argument buffer_template;
        unsigned int count;

        std::vector<std::uint8_t> min_values;
        std::vector<std::uint8_t> max_values;
        std::vector<std::uint8_t> data;
    };

    struct WorkData_IndexRange
    {
        unsigned int offset;
        unsigned int size;
    };

    struct WorkData_IndexBuffer
    {
        VkIndexType type;
        VkPrimitiveTopology topology;
        std::vector<WorkData_IndexRange> ranges;
        std::vector<std::uint8_t> data;
    };

    struct WorkData_Geometry
    {
        WorkData_IndexBuffer index_data;
        std::vector<std::uint8_t> assembled_index_buffer;

        std::vector<WorkData_VertexBuffer> vertex_data;
        std::vector<std::uint8_t> assembled_vertex_buffer;
    };
    int assemble_vertex_buffer(WorkData_Geometry& data);


    using WorkData_Mesh = std::unordered_map<std::string, WorkData_Geometry>;
    int parse_geometry(const std::string& input_file, const VertexBuffer_Template& vertex_buffer_template, WorkData_Mesh& mesh);
    int write_geometry(const std::string& output_file, const WorkData_Mesh& mesh);

}
