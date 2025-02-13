#pragma once
#include <string>
#include <vector>

struct ShaderFile
{
	std::string file;
	
	bool hasVertexShader = false;
	std::vector<unsigned int> vertexShaderSPIRV;
	
	bool hasFragmentShader = false;
	std::vector<unsigned int> fragmentShaderSPIRV;

	ShaderFile(const std::string& filePath);
	ShaderFile() = default;

};
