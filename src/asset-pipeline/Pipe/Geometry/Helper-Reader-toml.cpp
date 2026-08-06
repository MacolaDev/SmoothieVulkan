//
// Created by macola on 7/3/26.
//

#include "Helper.h"
#include "include/toml.hpp"

using namespace Smoothie::AssetPipeline::Geometry_Helpers;

using toml_basic_value = toml::basic_value<toml::type_config>;

static std::string error_info_from_toml(const toml_basic_value& configuration)
{
    const auto& _location = configuration.location();
    return _location.file_name() + "(" + std::to_string(_location.first_line_number()) + "," + std::to_string(_location.first_column_number()) + ")";
}

static inline int parse_int(const toml_basic_value& value, unsigned int& out)
{
    if (value.is_integer())
    {
        out = value.as_integer();
        return 0;
    }
    return 1;
}

static int parse_string_array(const toml_basic_value& value, std::vector<std::string>& out)
{
    if (!value.is_array()) return 1;

    for (const auto& _value : value.as_array())
    {
        if (!_value.is_string()) return 1;

        out.push_back(_value.as_string());
    }

    return 0;
}

static int get_data_type_from_toml(const std::string& name, unsigned int count, Smoothie::Geometry_Data_Type& output)
{
    output.count = count;
    if (name == "int8")
    {
        output.base_type = Smoothie::Geometry_Data_Type::Int_8;
        return 0;
    }

    if (name == "int16")
    {
        output.base_type = Smoothie::Geometry_Data_Type::Int_16;
        return 0;
    }

    if (name == "int32")
    {
        output.base_type = Smoothie::Geometry_Data_Type::Int_32;
        return 0;
    }

    if (name == "int64")
    {
        output.base_type = Smoothie::Geometry_Data_Type::Int_64;
        return 0;
    }

    if (name == "uint8")
    {
        output.base_type = Smoothie::Geometry_Data_Type::UInt_8;
        return 0;
    }

    if (name == "uint16")
    {
        output.base_type = Smoothie::Geometry_Data_Type::UInt_16;
        return 0;
    }

    if (name == "uint32")
    {
        output.base_type = Smoothie::Geometry_Data_Type::UInt_32;
        return 0;
    }

    if (name == "uint64")
    {
        output.base_type = Smoothie::Geometry_Data_Type::UInt_64;
        return 0;
    }


    if (name == "float32")
    {
        output.base_type = Smoothie::Geometry_Data_Type::Float_32;
        return 0;
    }

    return 1;
}

static int parse_type(const toml_basic_value& value, Smoothie::Geometry_Data_Type& type)
{
    if (value.is_string())
    {
        return get_data_type_from_toml(value.as_string(), 1, type);
    }

    if (value.is_array() && (value.as_array().size() >= 2))
    {
        unsigned int _type_size = 0;
        const auto& _array = value.as_array();
        if (parse_int(_array.at(1), _type_size) != 0) return 1;

        return get_data_type_from_toml(_array[0].as_string(), _type_size, type);
    }
    return 1;
}

static Conversion_Function get_conversion_function(const toml_basic_value& value)
{
    const std::unordered_map<std::string, Conversion_Function> _lookup_table =
    {
        {"cast", Conversion_Function_cast},
        {"pack_uv", Conversion_Function_pack_uv},
        {"pack_normals_10_10_10_2", Conversion_Function_pack_normals_10_10_10_2}
    };

    if (_lookup_table.find(value.as_string()) != _lookup_table.end())
    {
        return _lookup_table.at(value.as_string());
    }
    fmt::print(R"(Unknown conversion function "{}"; Using default casting function "{}")", value.as_string(), error_info_from_toml(value));
    return Conversion_Function_cast;
}

static int parse_template_arguments(const toml_basic_value& attributes, VertexBuffer_Template_Argument& argument)
{

    if (!attributes.is_table())
    {
        fmt::print(R"(Configuration expects table of values in "{}")", error_info_from_toml(attributes));
        return 1;
    }

    const auto& _parameter_table = attributes.as_table();

    //Location
    if (_parameter_table.find("location") == _parameter_table.end())
    {
        fmt::print(R"(Unable to get required "location" specifier at: {})", error_info_from_toml(attributes));
        return 1;
    }
    if (parse_int(_parameter_table.at("location"), argument.location) != 0)
    {
        fmt::print(R"(Location must be an integer: {})", error_info_from_toml(attributes));
        return 1;
    }

    //Input names
    if (_parameter_table.find("input-names") == _parameter_table.end())
    {
        fmt::print(R"(Unable to get required "input-names" specifier at: {})", error_info_from_toml(attributes));
        return 1;
    }
    if (parse_string_array(_parameter_table.at("input-names"), argument.names) != 0)
    {
        fmt::print(R"(input-names must be an array of strings: {})", error_info_from_toml(_parameter_table.at("input-names")));
        return 1;
    }


    //Input type
    if (_parameter_table.find("input-type") == _parameter_table.end())
    {
        fmt::print(R"(input-type must be a valid type: {})", error_info_from_toml(attributes));
        return 1;
    }
    if (parse_type(_parameter_table.at("input-type"), argument.input) != 0)
    {
        fmt::print(R"(input-type must be a valid type: {})", error_info_from_toml(attributes));
        return 1;
    }


    //Output type
    if (_parameter_table.find("output-type") == _parameter_table.end())
    {
        fmt::print(R"(output-type must be a valid type: {})", error_info_from_toml(attributes));
        return 1;
    }
    if (parse_type(_parameter_table.at("output-type"), argument.output) != 0)
    {
        fmt::print(R"(output-type must be a valid type: {})", error_info_from_toml(attributes));
        return 1;
    }

    //Conversion
    if (const auto& _conversion = _parameter_table.find("conversion"); _conversion!= _parameter_table.end())
    {
        if (_conversion->second.is_string())
        {
            argument.conversion_function = get_conversion_function(_conversion->second);
            return 0;
        }
        fmt::print(R"(Conversion function must be a string: {})", error_info_from_toml(_parameter_table));
        return 1;
    }


    return 0;
}

static int parse_configuration(const toml_basic_value& configuration, VertexBuffer_Template& buffer_template)
{
    if (!configuration.is_array())
    {
        fmt::print(R"(Configuration expects array of values in: {})", error_info_from_toml(configuration));
        return 1;
    }

    VertexBuffer_Template_Argument _template_argument;
    for (const auto& _template_specifier: configuration.as_array())
    {
        if (parse_template_arguments(_template_specifier, _template_argument) != 0)
        {
            //fmt::print(R"(Failed to parse configuration for: {})", _template_specifier);
            continue;
        }
        buffer_template.push_back(std::move(_template_argument));
    }

    std::sort(buffer_template.begin(), buffer_template.end(), [](const VertexBuffer_Template_Argument& first, const VertexBuffer_Template_Argument& second)
    {
        return first.location < second.location;
    });

    unsigned int _offset = 0;
    for (auto& _data: buffer_template)
    {
        _data.offset = _offset;
        _data.size = get_type_size(_data.output.base_type) * _data.output.count;
        _offset += _data.size;
    }


    return 0;
}

int Smoothie::AssetPipeline::Geometry_Helpers::parse_vertex_buffer_templates(const std::string& input_file, VertexBuffer_Templates& templates)
{
    const auto& _file = toml::parse(input_file, toml::spec::v(1,1,0));
    if (!_file.is_table())
    {
        fmt::print(R"(Invalid configuration file!: {})", input_file);
        return 1;
    }

    const auto& _configuration_table = _file.as_table();
    for (const auto& [_name, _configuration] : _configuration_table)
    {
        VertexBuffer_Template buffer_template;
        if (parse_configuration(_configuration, buffer_template) != 0)
        {
            fmt::print(R"(Failed to parse configuration for: {})", _name);
            return 1;
        }
        templates.insert({_name, std::move(buffer_template)});
    }
    return 0;
}

