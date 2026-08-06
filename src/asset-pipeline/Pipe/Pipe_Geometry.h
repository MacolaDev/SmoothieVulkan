#pragma once
#include "Pipe_Base.h"

namespace Smoothie::AssetPipeline
{
    struct Pipe_Geometry : public Pipe_Base
    {
        void process(const std::string &input_file, const std::string &output_file,
            const std::vector<std::string> &processing_options) override;

        std::string_view help_message() const noexcept override;
    };

}