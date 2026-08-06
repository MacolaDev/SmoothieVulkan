//
// Created by macola on 7/6/26.
//
#include "Helper.h"
#include <algorithm>

using namespace Smoothie::AssetPipeline::Geometry_Helpers;

void Smoothie::AssetPipeline::Geometry_Helpers::Conversion_Function_cast(
    std::vector<std::uint8_t>& data,
    const VertexBuffer_Template_Argument& vertex_template)
{

}

void Smoothie::AssetPipeline::Geometry_Helpers::Conversion_Function_pack_uv(
    std::vector<std::uint8_t>& data,
    const VertexBuffer_Template_Argument& vertex_template)
{

}

void Smoothie::AssetPipeline::Geometry_Helpers::Conversion_Function_pack_normals_10_10_10_2(
    std::vector<std::uint8_t>& data,
    const VertexBuffer_Template_Argument& vertex_template)
{

}

int Smoothie::AssetPipeline::Geometry_Helpers::assemble_vertex_buffer(WorkData_Geometry& data)
{
    if (data.vertex_data.empty()) return 0;

    std::sort(data.vertex_data.begin(), data.vertex_data.end(), [](const WorkData_VertexBuffer& first, const WorkData_VertexBuffer& second)
        {
            return first.buffer_template.location < second.buffer_template.location;
        });


    const auto& _vertex_data = data.vertex_data;

    unsigned int _vertex_count = _vertex_data.at(0).count;
    for (unsigned int i = 1; i < _vertex_data.size(); ++i)
    {
        if (_vertex_count != _vertex_data.at(i).count)
        {
            fmt::print(R"(Expected vertex count for attribute: {}, but got: {}; can't create vertex buffer. )", _vertex_count, _vertex_data.at(i).count);
            return 1;
        }
    }


    unsigned int _vertex_buffer_stride = _vertex_data.back().buffer_template.size + _vertex_data.back().buffer_template.offset;
    unsigned int _vertex_buffer_size = _vertex_count * _vertex_buffer_stride;
    data.assembled_vertex_buffer.resize(_vertex_buffer_size);

    for (unsigned int attribute_index = 0; attribute_index < _vertex_data.size(); attribute_index++)
    {
        const auto& _template = _vertex_data[attribute_index].buffer_template;
        const auto& _offset = _template.offset;
        const auto& _size = _template.size;
        const auto& _data = _vertex_data[attribute_index].data;

        unsigned int _buffer_offset = 0;
        for (unsigned int vertex_id = 0; vertex_id < _vertex_count; vertex_id++)
        {
            _buffer_offset = _offset + vertex_id * _vertex_buffer_stride;
            std::memcpy(&data.assembled_vertex_buffer[_buffer_offset], &_data[vertex_id * _size], _size);
        }
    }

    // std::ofstream out("/home/macola/Projects/SmoothieVulkan/bin/testing.bin");
    // out.write(reinterpret_cast<const std::ostream::char_type *>(data.assembled_vertex_buffer.data()), data.assembled_vertex_buffer.size());
    // out.close();

    return 0;
}