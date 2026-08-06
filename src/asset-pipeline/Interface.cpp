//
// Created by macola on 6/24/26.
//

#include "Interface.h"

#include <iostream>

#include "Pipe/Pipe_Geometry.h"
#include "Pipe/Pipe_Shader.h"
#include "Pipe/Pipe_Text.h"

using namespace Smoothie::AssetPipeline;

void Smoothie::AssetPipeline::select_and_run_pipeline(
    const std::string& pipe,
    const std::string& input_file,
    const std::string& output_file,
    const std::vector<std::string>& arguments)
{
    if (pipe == "shader")
    {
        Pipe_Shader pipe_shader;
        pipe_shader.process(input_file, output_file, arguments);
        return;
    }

    if (pipe == "text")
    {
        Pipe_Text pipe_text;
        pipe_text.process(input_file, output_file, arguments);
        return;
    }

    if (pipe == "geometry")
    {
        Pipe_Geometry pipe_geometry;
        pipe_geometry.process(input_file, output_file, arguments);
        return;
    }
}


void Smoothie::AssetPipeline::message(const std::string_view& message)
{
    std::cout << message << std::endl;
}