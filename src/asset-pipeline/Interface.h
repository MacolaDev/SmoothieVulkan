#pragma once
#include "Common.h"

namespace Smoothie::AssetPipeline
{
    void select_and_run_pipeline(
        const std::string& pipe,
        const std::string& input_file,
        const std::string& output_file,
        const std::vector<std::string>& arguments);

    void message(const std::string_view& message);
}