#pragma once
#include <string>
#include <vector>

class GeometryFile
{
protected:
	std::string filepath;
	std::vector<char> vertexData;
	std::vector<char> indexData;

	unsigned int numberOfVertices;
	unsigned int numberOfIndices;
	unsigned int vertexTypeLenght;
	
	unsigned int vertexBufferType;
	unsigned int indexBufferType;


public:
	GeometryFile(const std::string& filepath);
	GeometryFile() = default;
};

