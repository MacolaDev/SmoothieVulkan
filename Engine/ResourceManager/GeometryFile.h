#pragma once
#include "Core/Common.h"

struct GeometryFile
{
	std::vector<char> vertexData;
	std::vector<char> indexData;

	unsigned int numberOfVertices;
	unsigned int numberOfIndices;
	unsigned int vertexTypeLenght;
	
	unsigned int vertexBufferType;
	unsigned int indexBufferType;

	GeometryFile(const std::string& filepath);
	GeometryFile() = default;
};

