//
// Created by macola on 6/24/26.
//

#include "Pipe_Geometry.h"
#include "Interface.h"
#include "Geometry/Helper.h"


void Smoothie::AssetPipeline::Pipe_Geometry::process(const std::string &input_file, const std::string &output_file, const std::vector<std::string> &processing_options)
{
    //1. Parse input arguments
    std::string _configuration_file;
    std::string _configuration;
    for (const auto &_option : processing_options)
    {
        size_t pos = _option.find('=');
        if (pos == std::string::npos) continue;

        const std::string& _key = _option.substr(0, pos);
        const std::string& _value = _option.substr(pos + 1);

        if (_key == "--templates-file")
        {
            _configuration_file = _value;
            continue;
        }

        if (_key == "--template")
        {
            _configuration = _value;
            continue;
        }
    }


    Geometry_Helpers::VertexBuffer_Templates vertex_buffer_templates;
    if (Geometry_Helpers::parse_vertex_buffer_templates(_configuration_file, vertex_buffer_templates) != 0)
    {
        fmt::print(R"(Failed to parse configuration file: {})", _configuration_file);
        return;
    }

    if (vertex_buffer_templates.find(_configuration) == vertex_buffer_templates.end())
    {
        fmt::print(R"(No vertex buffer template named "{}")", _configuration);
        return;
    }
    const auto& _selected_template = vertex_buffer_templates.at(_configuration);

    Geometry_Helpers::WorkData_Mesh mesh;
    if (Geometry_Helpers::parse_geometry(input_file, _selected_template, mesh) != 0)
    {
        fmt::print(R"(Failed to parse geometry file: {})", input_file);
        return;
    }

    for (auto& [key, value] : mesh)
    {
        if (Geometry_Helpers::assemble_vertex_buffer(value) != 0)
        {
            fmt::print(R"(Failed to assemble vertex buffer for "{}")", key);
            mesh.erase(key);
        }

        //For now...
        value.assembled_index_buffer = std::move(value.index_data.data);

    }

    if (Geometry_Helpers::write_geometry(output_file, mesh) != 0)
    {
        fmt::print(R"(Failed to write geometry file: {})", output_file);
    }

}

std::string_view Smoothie::AssetPipeline::Pipe_Geometry::help_message() const noexcept
{
    return R"()";
}
