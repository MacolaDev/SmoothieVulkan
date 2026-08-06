//
// Created by macola on 6/19/26.
//

#include <fstream>

#include "asset-pipeline.h"

#include <iostream>

#include "Pipe/Pipe_Shader.h"

static constexpr std::string_view valid_usage() noexcept
{
    return R"(Usage: asset-pipeline [pipe] [pipe-options] --input-file [file] --output-file [file]

Where [pipe] must be one of:
    text
    shader
    geometry
    image

Where [pipe-options] can be 0 or more options for a specific [pipe].
To query possible options for a specific [pipe] call:

    asset-pipeline [pipe] --options

Other options:
    --input-file   [file]		Path to input file.
    --output-file  [file]		Path to output file.

Example usage: asset-pipeline geometry --input-file source.fbx --output-file source.sgeometry)";
}

static bool pipe_value_supported(const char* _pipe)
{
    if (_pipe == nullptr) return false;

    if (std::strcmp(_pipe, "text") == 0) return true;
    if (std::strcmp(_pipe, "shader") == 0) return true;
    if (std::strcmp(_pipe, "geometry") == 0) return true;
    if (std::strcmp(_pipe, "image") == 0) return true;

    return false;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "[pipe] not provided!" << std::endl;
        std::cout << valid_usage() << std::endl;
        return 0;
    }

    const char* _pipe = argv[1];
    if (!pipe_value_supported(_pipe))
    {
        std::cerr << "Unsupported value for [pipe]: " << _pipe << std::endl;
        std::cout << valid_usage() << std::endl;
        return 0;
    }


    std::vector<std::string> _arguments;
    std::string _input_file;
    std::string _output_file;
    for (int i = 2; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--input-file") == 0)
        {
            i += 1;
            if (i < argc)
            {
                _input_file = argv[i];
                continue;
            }
            std::cerr << "[path] for \"--input-file\" not provided!" << std::endl;
            std::cout << valid_usage() << std::endl;
            return 0;
        }

        if (std::strcmp(argv[i], "--output-file") == 0)
        {
            i += 1;
            if (i < argc)
            {
                _output_file = argv[i];
                continue;
            }
            std::cerr << "[path] for \"--output-file\" not provided!" << std::endl;
            std::cout << valid_usage() << std::endl;
            return 0;
        }


        _arguments.emplace_back(argv[i]);
    }

    //std::cout << "Processing file: " << _input_file;
    try
    {
        Smoothie::AssetPipeline::select_and_run_pipeline(_pipe, _input_file, _output_file, _arguments);
    } catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
   // std::cout << " ...done." << std::endl;

    return 0;
}
