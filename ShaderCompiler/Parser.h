#pragma once
#include <string>
#include "glslang/Public/ShaderLang.h"

struct InputArguments
{
	static std::string compilerFile;
	static std::string sourceFile;
	static std::string destFile;
};

class SmoothieIncluder: public glslang::TShader::Includer
{
public:
	IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override;
	IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override;
private:

};

