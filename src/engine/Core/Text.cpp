//
// Created by macola on 6/21/26.
//

#include "Text.h"
#include "SmoothieCore.h"
#include <filesystem>
#include <fstream>

using namespace Smoothie;

int Smoothie::TextFile::load(const std::string &filepath)
{
    if (!std::filesystem::exists(filepath))
    {
        SmoothieCore::logError_Formated("No file named: {}", filepath);
        return 1;
    }

    std::ifstream file(filepath);


    file.close();
    return 0;
}
