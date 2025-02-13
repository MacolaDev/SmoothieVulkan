#include "Parser.h"

using namespace glslang;


TShader::Includer::IncludeResult* SmoothieIncluder::includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth)
{
	return nullptr;
}

TShader::Includer::IncludeResult* SmoothieIncluder::includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth)
{
	return nullptr;
}

void SmoothieIncluder::releaseInclude(IncludeResult*)
{
}
