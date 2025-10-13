#include "GeometryFile.h"
#include <fstream>
#include "Core/VertexBuffer.h"

static inline int getIntFromFile(std::ifstream& file) {
	char buffer[4];
	file.read(buffer, 4);
	int value = *(int*)buffer;
	return value;
}

using namespace Depricated;
GeometryFile::GeometryFile(const std::string& filepath) : indexBufferType(0)
{
	auto file = std::ifstream(filepath, std::ios_base::binary);

	vertexBufferType = getIntFromFile(file);
	numberOfVertices = getIntFromFile(file);
	int vertexOffset = getIntFromFile(file);

	numberOfIndices = getIntFromFile(file);
	int indexOffset = getIntFromFile(file);
	
	vertexTypeLenght = 0;
	if (vertexBufferType == 0) 
	{
		vertexTypeLenght = sizeof(xyznuvtb) - sizeof(VertexBufferBase);
	}
	if (vertexBufferType == 1)
	{
		vertexTypeLenght = sizeof(xyznuvtbc) - sizeof(VertexBufferBase);
	}

	file.seekg(vertexOffset);

	for(unsigned int i = 0; i < numberOfVertices * vertexTypeLenght; i++)
	{
		vertexData.push_back(file.get());
	}
		
	file.seekg(indexOffset);
	for (unsigned int i = 0; i < numberOfIndices * sizeof(unsigned int); i++)
	{
		indexData.push_back(file.get());
	}

	file.close();
}
