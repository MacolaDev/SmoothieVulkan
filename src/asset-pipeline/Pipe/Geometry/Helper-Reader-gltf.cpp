//
// Created by macola on 7/6/26.
//

#include "Helper.h"

#define TINYGLTF3_IMPLEMENTATION
#define TINYGLTF3_ENABLE_FS
#include "tiny_gltf_v3.h"

using namespace Smoothie::AssetPipeline::Geometry_Helpers;

namespace
{
    struct GLTF_Buffers_Accessors
    {
        std::vector<tg3_buffer> buffers;
        std::vector<tg3_buffer_view> buffer_views;
        std::vector<tg3_accessor> accessors;
    };
}

static int find_template_by_name(const std::string& accessor_name, const VertexBuffer_Template& selected_template, unsigned int& result_index)
{
    result_index = 0;
    unsigned int _counter = 0;
    for (const auto& _argument: selected_template)
    {
        for (const auto& _name : _argument.names)
        {
            if (_name == accessor_name)
            {
                result_index = _counter;
                return 0;
            }
        }
        _counter++;
    }

    fmt::print(R"(Warning: Unable to find matching names for: {})", accessor_name);
    return 1;
}

static bool check_input_types(int type, int component_type, const WorkData_VertexBuffer& result)
{
    int _n_of_components = tg3_num_components(type);

    Smoothie::Geometry_Data_Type::Base_Type _base;
    switch (component_type)
    {
        case TG3_COMPONENT_TYPE_BYTE: _base = Smoothie::Geometry_Data_Type::Base_Type::Int_8; break;
        case TG3_COMPONENT_TYPE_UNSIGNED_BYTE: _base = Smoothie::Geometry_Data_Type::Base_Type::UInt_8; break;
        case TG3_COMPONENT_TYPE_SHORT: _base = Smoothie::Geometry_Data_Type::Base_Type::Int_16; break;
        case TG3_COMPONENT_TYPE_UNSIGNED_SHORT: _base = Smoothie::Geometry_Data_Type::Base_Type::UInt_16; break;
        case TG3_COMPONENT_TYPE_INT: _base = Smoothie::Geometry_Data_Type::Base_Type::Int_32; break;
        case TG3_COMPONENT_TYPE_UNSIGNED_INT: _base = Smoothie::Geometry_Data_Type::Base_Type::UInt_32; break;
        case TG3_COMPONENT_TYPE_FLOAT: _base = Smoothie::Geometry_Data_Type::Base_Type::Float_32; break;
        default: _base = Smoothie::Geometry_Data_Type::Base_Type::Unknown; break;
    }

    return (_n_of_components == result.buffer_template.input.count) && (_base == result.buffer_template.input.base_type);
}

static int parse_mesh_vertex_data(
    const tg3_mesh& mesh,
    const VertexBuffer_Template& vertex_template,
    const GLTF_Buffers_Accessors& input_data,
    std::vector<WorkData_VertexBuffer>& output)
{
    if (mesh.primitives_count == 0) return 1;
    if (mesh.primitives_count > 1) return 1;

    const auto& _primitive = mesh.primitives[0];


    for (int i = 0; i < _primitive.attributes_count; i++)
    {
        const auto& _attribute_name = _primitive.attributes[i].key;
        const auto& _attribute_index = _primitive.attributes[i].value;

        unsigned int result_index = 0;
        if (find_template_by_name(std::string(_attribute_name.data, _attribute_name.len), vertex_template, result_index) != 0) continue;

        WorkData_VertexBuffer _res = {};
        _res.buffer_template = vertex_template[result_index];

        const auto& _accessor = input_data.accessors[_attribute_index];

        if (!check_input_types(_accessor.type, _accessor.component_type, _res)) continue;

        const auto& _buffer_view = input_data.buffer_views[_accessor.buffer_view];
        const auto& _buffer_offset = _accessor.byte_offset + _buffer_view.byte_offset;
        const auto& _buffer_size = tg3_accessor_byte_stride(&_accessor, &_buffer_view) * _accessor.count;


        if (_buffer_size <= 0) return 1;

        const auto& _buffer = input_data.buffers[_buffer_view.buffer];

        if (_buffer_offset + _buffer_size > _buffer.byte_length)
        {
            fmt::print(R"([DEBUG] _buffer_offset + _buffer_size > _buffer.byte_length ?)");
            continue;
        }
        _res.count = _accessor.count;
        _res.data.resize(_buffer_size);
        std::memcpy(&_res.data[0], _buffer.data.data + _buffer_offset, _buffer_size);

        output.push_back(std::move(_res));
    }

    return 0;
}

static int get_primitive_mode(int32_t mode, VkPrimitiveTopology& out)
{
    switch (mode)
    {
        case TG3_MODE_POINTS: out = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; return 0;
        case TG3_MODE_LINE: out = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; return 0;
        case TG3_MODE_LINE_STRIP: out = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP; return 0;
        case TG3_MODE_TRIANGLES: out = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; return 0;
        case TG3_MODE_TRIANGLE_STRIP: out = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; return 0;
        case TG3_MODE_TRIANGLE_FAN: out = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN; return 0;
        default: out = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM; return 1;
    }
}

static int get_primitive_type(int32_t component_type, VkIndexType& out)
{
    switch (component_type)
    {
        case TG3_COMPONENT_TYPE_BYTE: out = VK_INDEX_TYPE_UINT8; return 0;
        case TG3_COMPONENT_TYPE_UNSIGNED_BYTE: out = VK_INDEX_TYPE_UINT8; return 0;
        case TG3_COMPONENT_TYPE_SHORT: out = VK_INDEX_TYPE_UINT16; return 0;
        case TG3_COMPONENT_TYPE_UNSIGNED_SHORT: out = VK_INDEX_TYPE_UINT16; return 0;
        case TG3_COMPONENT_TYPE_INT: out = VK_INDEX_TYPE_UINT32; return 0;
        case TG3_COMPONENT_TYPE_UNSIGNED_INT: out = VK_INDEX_TYPE_UINT32; return 0;
        default: out = VK_INDEX_TYPE_MAX_ENUM; return 1;
    }
}

static int parse_mesh_index_data(const tg3_mesh& mesh, const GLTF_Buffers_Accessors& input_data, WorkData_IndexBuffer& output)
{
    if (mesh.primitives_count == 0) return 1;
    if (mesh.primitives_count > 1) fmt::print(R"([DEBUG] More primitives in one mesh ?)");

    const auto& _primitive = mesh.primitives[0];
    output.ranges.clear();
    const auto& _index_accessor = input_data.accessors[_primitive.indices];
    WorkData_IndexRange _range{};
    _range.offset = 0;
    _range.size = _index_accessor.count;
    output.ranges.push_back(_range);

    if (get_primitive_mode(_primitive.mode, output.topology) != 0)
    {
        fmt::print(R"(Invalid topology for a mesh!)");
        return 1;
    }

    if (get_primitive_type(_index_accessor.component_type, output.type))
    {
        fmt::print(R"(Invalid primitive type for a mesh!)");
        return 1;
    }

    const auto& _buffer_view = input_data.buffer_views[_index_accessor.buffer_view];
    const auto& _buffer_offset = _index_accessor.byte_offset + _buffer_view.byte_offset;
    const auto& _buffer_size = tg3_accessor_byte_stride(&_index_accessor, &_buffer_view) * _index_accessor.count;

    const auto& _buffer = input_data.buffers[_buffer_view.buffer];
    if (_buffer_offset + _buffer_size > _buffer.byte_length)
    {
        fmt::print(R"([DEBUG] _buffer_offset + _buffer_size > _buffer.byte_length ? )");
        return 0;
    }

    output.data.clear();
    output.data.resize(_buffer_size);
    std::memcpy(&output.data[0], _buffer.data.data + _buffer_offset, _buffer_size);

    return 0;
}

int Smoothie::AssetPipeline::Geometry_Helpers::parse_geometry(
    const std::string& input_file,
    const VertexBuffer_Template& vertex_buffer_template,
    WorkData_Mesh& mesh)
{
    tg3_parse_options opts;
    tg3_error_stack errors;
    tg3_model model;
    tg3_parse_options_init(&opts);
    tg3_error_stack_init(&errors);

    if (tg3_parse_file(&model, &errors, input_file.c_str(), 10, &opts) != TG3_OK)
    {
        for (uint32_t i = 0; i < errors.count; i++)
        {
            fmt::print(R"([{}] {})", (int)errors.entries[i].severity, ((errors.entries[i].message != nullptr) ? errors.entries[i].message : "nullptr!"));
        }

        tg3_model_free(&model);
        tg3_error_stack_free(&errors);
        return 1;
    }

    if ((model.buffers == nullptr) || (model.buffer_views == nullptr) || (model.accessors == nullptr)) return 1;

    GLTF_Buffers_Accessors _buffer_data;
    _buffer_data.buffers.resize(model.buffers_count);
    for (unsigned int i = 0; i < model.buffers_count; i++)
    {
        _buffer_data.buffers[i] = model.buffers[i];
    }
    _buffer_data.buffer_views.resize(model.buffer_views_count);
    for (unsigned int i = 0; i < model.buffer_views_count; i++)
    {
        _buffer_data.buffer_views[i] = model.buffer_views[i];
    }

    _buffer_data.accessors.resize(model.accessors_count);
    for (unsigned int i = 0; i < model.accessors_count; i++)
    {
        _buffer_data.accessors[i] = model.accessors[i];
    }

    for (auto i = 0; i < model.meshes_count; i++)
    {
        const auto& _mesh = model.meshes[i];
        WorkData_Geometry _out;
        if (parse_mesh_vertex_data(_mesh, vertex_buffer_template, _buffer_data, _out.vertex_data) != 0)
        {
            fmt::print(R"(Failed to get vertex data for "{}")", _mesh.name.data);
            continue;
        }

        if (parse_mesh_index_data(_mesh, _buffer_data, _out.index_data) != 0)
        {
            fmt::print(R"(Failed to get index data for "{}")", _mesh.name.data);
            continue;
        }

        mesh.insert({std::string(_mesh.name.data, _mesh.name.len), std::move(_out)});
    }


    tg3_model_free(&model);
    tg3_error_stack_free(&errors);
    return 0;
}