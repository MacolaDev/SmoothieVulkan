#pragma once
#include <string>

struct ShaderFile
{
	std::string file;

	std::string vertexShader;
	std::string fragmentShader;

	bool hasGeometryShader;
	std::string geometryShader;


	ShaderFile(const std::string& filePath);
	ShaderFile() = default;

};
