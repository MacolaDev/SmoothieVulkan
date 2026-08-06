#pragma once
#include "Common.h"

namespace Smoothie::AssetPipeline
{
    struct Pipe_Base
    {
        virtual void process(
            const std::string& input_file,
            const std::string& output_file,
            const std::vector<std::string>& processing_options) = 0;

        virtual std::string_view help_message() const noexcept = 0;
        

        virtual ~Pipe_Base() = default;
    };

}