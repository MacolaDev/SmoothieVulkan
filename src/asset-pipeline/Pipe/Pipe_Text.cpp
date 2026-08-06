//
// Created by macola on 6/22/26.
//

#include "Pipe_Text.h"

#include "single_include/toml.hpp"

using namespace Smoothie::AssetPipeline;
using namespace Smoothie;

static std::unique_ptr<Text_Node_Base> process_bool(bool b)
{
    auto result = std::make_unique<Text_Node_Int32>();
    result->data = static_cast<int>(b);
    return result;
}

static std::unique_ptr<Text_Node_Base> process_int(int i)
{
    auto result = std::make_unique<Text_Node_Int32>();
    result->data = i;
    return result;
}

static std::unique_ptr<Text_Node_Base> process_float(float f)
{
    auto result = std::make_unique<Text_Node_Float32>();
    result->data = f;
    return result;
}

static std::unique_ptr<Text_Node_Base> process_string(std::string_view s)
{
    auto result = std::make_unique<Text_Node_String>();
    result->data = s;
    return result;
}

static bool check_array_support(const std::vector<toml::basic_value<toml::type_config>>& in_data, toml::value_t _type)
{
    auto _f_check_support = [&_type](const toml::basic_value<toml::type_config>& element)
    {
        if (element.type() == _type)
        {
            return true;
        }
        return false;
    };

    return std::all_of(in_data.cbegin(), in_data.cend(), _f_check_support);
}

static std::unique_ptr<Text_Node_Base> process_array(const std::vector<toml::basic_value<toml::type_config>>& in_data)
{
    auto result = std::make_unique<Text_Node_Array_String>();
    //result->data = s;
    return result;
}

static int build_text(const toml::basic_value<toml::type_config>& _toml_document, Smoothie::Text_Map& new_map)
{

    if (!_toml_document.is_table())
    {
        fmt::print("Input type must be a table.");
        return 1;
    }
    const auto& _document_table = _toml_document.as_table();



    for (const auto& [key, value] : _document_table)
    {
        if (value.type() == toml::value_t::empty) continue;

        if (value.type() == toml::value_t::boolean)
        {
            new_map.insert({key, process_bool(value.as_boolean())});
            continue;
        }

        if (value.type() == toml::value_t::integer)
        {
            new_map.insert({key, process_int(value.as_integer())});
            continue;
        }

        if (value.type() == toml::value_t::floating)
        {
            new_map.insert({key, process_int(value.as_floating())});
            continue;
        }

        if (value.type() == toml::value_t::string)
        {
            new_map.insert({key, process_string(value.as_string())});
            continue;
        }

        if (value.type() == toml::value_t::array)
        {
            const auto& _array = value.as_array();
            if (check_array_support(_array, toml::value_t::string))
            {

            }


            //new_map.insert({key, process_array(value.as_array())});
            continue;
        }

    }


    return 0;
}


void Pipe_Text::process(const std::string &input_file, const std::string &output_file, const std::vector<std::string> &processing_options)
{
    toml::basic_value<toml::type_config> _toml_document = toml::parse(input_file, toml::spec::v(1,1,0));

    Text_Map _text_map;
    build_text(_toml_document, _text_map);

    // const std::function f_parse_string = [&main_table](const std::string& key, const std::string& value)
    // {
    //
    // };
    //
    // std::ofstream _output(output_file.c_str(), std::ios::out);
    //
    // for (const auto& [key, value]: main_table)
    // {
    //     std::cout << key << ": " << value << std::endl;
    //     if (value.is_string())
    //     {
    //
    //     }
    //
    // }
    //
    // _output.close();

}

std::string_view Pipe_Text::help_message() const noexcept
{
    return
R"(Text processing. Converts .toml files into binary data that engine can read with "Smoothie::TextFile" class.

    No options for this type (at least for now :D)

)";
}
