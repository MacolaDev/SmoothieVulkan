#include "ShaderFile.h"
#include <fstream>
#include <sstream>
#include <iostream>

static inline std::string getSourceCodeFromFile(const std::string filepath)
{
	std::string fileContent;
	std::ifstream file(filepath);

	if (file.is_open()) {
		std::stringstream buffer;

		buffer << file.rdbuf();
		fileContent = buffer.str();

		file.close();
	}
	else
	{
		std::cerr << __FUNCTION__": Unable to open the file: " + filepath << std::endl;
	}
	return fileContent;
}

static std::string insertPreprocessorStatment(std::string source, const std::string& preprocessorStatment)
{

	auto index = source.find("#ifdef");
	return source.insert(index, "\n" + preprocessorStatment + "\n");
}

ShaderFile::ShaderFile(const std::string& filePath)
{
	std::string  sourceCode = getSourceCodeFromFile(filePath);

	vertexShader = insertPreprocessorStatment(sourceCode, "#define VERTEX_SHADER");
	fragmentShader = insertPreprocessorStatment(sourceCode, "#define FRAGMENT_SHADER");


	if (sourceCode.find("#ifdef GEOMETRY_SHADER") != -1)
	{
		hasGeometryShader = true;
		geometryShader = insertPreprocessorStatment(sourceCode, "#define GEOMETRY_SHADER");
	}
	else
	{
		hasGeometryShader = false;
	}

}
