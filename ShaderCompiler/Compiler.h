#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <fstream>

#include "Smoothie.h"

#include "slang/slang.h"

namespace SmoothieCompiler
{

    struct Settings
    {
        //Source code input file for the compiler.
        std::string input_file;

        //Output file.
        std::string output_file;

        std::string compiler_file;

    };

    class CompiledShader: public Smoothie::ShaderFile
    {
    protected:
        virtual int get_data(slang::VariableLayoutReflection* layout, Smoothie::Variable& variable);
        virtual int get_data(slang::VariableLayoutReflection* layout, Smoothie::Variable_Global& variable);
        virtual int get_data(slang::TypeLayoutReflection* layout);
        virtual int get_data(slang::TypeLayoutReflection* layout, std::shared_ptr<Smoothie::Shader_Type_Uniform>& newType);
        virtual int get_data(slang::TypeLayoutReflection* layout, std::shared_ptr<Smoothie::Shader_Type_Struct>& newType);
        virtual int get_data(slang::TypeLayoutReflection* layout, std::shared_ptr<Smoothie::Shader_Type_Resource>& newType);

    public:
        Settings m_CompilerSettings;

        int serialize(std::ofstream& data) override;
        int compile_shader();
    };


}