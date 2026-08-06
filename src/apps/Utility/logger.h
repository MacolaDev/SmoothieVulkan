#pragma once
#include "../Smoothie.h"
namespace Smoothie::App_Utilities
{
    void setup_logging(const std::string& out_file, bool logVerbose, bool logPerformance);
    void logger(const Smoothie::Debug_Info& info);
}