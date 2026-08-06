//
// Created by macola on 7/18/26.
//
#include "logger.h"

#include "fmt/color.h"
#include "fmt/printf.h"

static std::ofstream g_OutputFile;
static bool g_LogVerbose = false;
static bool g_LogPerformance = false;
void Smoothie::App_Utilities::setup_logging(const std::string& out_file, bool logVerbose, bool logPerformance)
{
    g_LogVerbose = logVerbose;
    g_LogPerformance = logPerformance;
    if (!out_file.empty())
    {
        assert(!g_OutputFile.is_open());
        g_OutputFile.open(out_file);
    }
}

void Smoothie::App_Utilities::logger(const Debug_Info& info)
{
    std::string_view _msg_header;
    fmt::color _color;
    switch (info.severity)
    {
        case Debug_Info::Message_Severity_VERBOSE:      _msg_header="[VERBOSE]",       _color = fmt::color::aqua;          break;
        case Debug_Info::Message_Severity_INFO:         _msg_header="[INFO]",          _color = fmt::color::green_yellow;  break;
        case Debug_Info::Message_Severity_WARNING:      _msg_header="[WARNING]",       _color = fmt::color::yellow;        break;
        case Debug_Info::Message_Severity_ERROR:        _msg_header="[ERROR]",         _color = fmt::color::red;           break;
        case Debug_Info::Message_Severity_CRITICAL:     _msg_header="[CRITICAL]",      _color = fmt::color::red;           break;
        case Debug_Info::Message_Severity_PERFORMANCE:  _msg_header="[PERFORMANCE]",   _color = fmt::color::orange;        break;
        default: return;
    }

    if (!g_LogVerbose     && (_msg_header == "[VERBOSE]")) return;
    if (!g_LogPerformance && (_msg_header == "[PERFORMANCE]")) return;

    if (g_OutputFile.is_open())
    {
        fmt::print(g_OutputFile, "{} {}\n", _msg_header, info.message);
        g_OutputFile << fmt::format("{} {}\n", _msg_header, info.message);
    }
    fmt::print(fmt::fg(_color),_msg_header);
    fmt::print(" {}\n", info.message);
}