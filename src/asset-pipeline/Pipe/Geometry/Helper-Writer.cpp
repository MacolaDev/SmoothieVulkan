//
// Created by macola on 7/8/26.
//

#include "Helper.h"

using namespace Smoothie::AssetPipeline::Geometry_Helpers;

static void write_buffer(std::ofstream& file, const std::vector<std::uint8_t>& buffer)
{
    unsigned int _buffer_size = buffer.size();
    file.write(reinterpret_cast<const std::ostream::char_type *>(&_buffer_size), sizeof(_buffer_size));
    if (_buffer_size > 0)
    {
        file.write(reinterpret_cast<const std::ostream::char_type *>(&buffer[0]), _buffer_size);
    }
}

static void write_string(std::ofstream& file, const std::string& data)
{
    unsigned int _string_size = data.size();
    file.write(reinterpret_cast<const char*>(&_string_size), sizeof(_string_size));
    file.write(data.data(), _string_size);
}

static void write_type(std::ofstream& file, const Smoothie::Geometry_Data_Type& data)
{
    file.write(reinterpret_cast<const char*>(&data.base_type), sizeof(data.base_type));
    file.write(reinterpret_cast<const char*>(&data.count), sizeof(data.count));
}

static void write_mesh(std::ofstream& file, const WorkData_Geometry& geometry)
{
    file.write(reinterpret_cast<const std::ostream::char_type *>(&geometry.index_data.type), sizeof(geometry.index_data.type));
    file.write(reinterpret_cast<const std::ostream::char_type *>(&geometry.index_data.topology), sizeof(geometry.index_data.topology));

    unsigned int _vertex_data_size = geometry.vertex_data.size();
    file.write(reinterpret_cast<const char*>(&_vertex_data_size), sizeof(_vertex_data_size));

    for (const auto& vertex_data : geometry.vertex_data)
    {
        file.write(reinterpret_cast<const std::ostream::char_type *>(&vertex_data.buffer_template.offset), sizeof(vertex_data.buffer_template.offset));
        file.write(reinterpret_cast<const std::ostream::char_type *>(&vertex_data.buffer_template.size), sizeof(vertex_data.buffer_template.size));

        write_type(file, vertex_data.buffer_template.output);
        write_type(file, vertex_data.buffer_template.input);
        write_buffer(file, vertex_data.min_values);
        write_buffer(file, vertex_data.max_values);
    }

    unsigned int _ranges = geometry.index_data.ranges.size();
    file.write(reinterpret_cast<const std::ostream::char_type *>(&_ranges), sizeof(_ranges));
    for (const auto& _range : geometry.index_data.ranges)
    {
        std::uint32_t vertex_offset = 0;
        std::uint32_t vertex_count = 0;
        std::uint32_t index_offset = _range.offset;
        std::uint32_t index_count = _range.size;

        file.write(reinterpret_cast<const std::ostream::char_type *>(&vertex_offset), sizeof(vertex_offset));
        file.write(reinterpret_cast<const std::ostream::char_type *>(&vertex_count), sizeof(vertex_count));
        file.write(reinterpret_cast<const std::ostream::char_type *>(&index_offset), sizeof(index_offset));
        file.write(reinterpret_cast<const std::ostream::char_type *>(&index_count), sizeof(index_count));
    }

    write_buffer(file, geometry.assembled_vertex_buffer);
    write_buffer(file, geometry.assembled_index_buffer);
}


int Smoothie::AssetPipeline::Geometry_Helpers::write_geometry(const std::string& output_file, const WorkData_Mesh& mesh)
{
    std::ofstream out(output_file, std::ios::binary);

    out.write(Smoothie::GeometryFile::get_Magic(), 4);

    unsigned int _n_of_meshes = mesh.size();
    out.write(reinterpret_cast<const std::ostream::char_type *>(&_n_of_meshes), sizeof(_n_of_meshes));
    for (const auto& [_name, _mesh] : mesh)
    {
        write_string(out, _name);
        write_mesh(out, _mesh);
    }

    out.close();
    return 0;
}