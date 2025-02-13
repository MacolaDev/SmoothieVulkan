#include "ShaderFile.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

struct FileHeader
{
	unsigned int type = 0;
	unsigned int size = 0;
	unsigned int ptrInFile = 0;
};

ShaderFile::ShaderFile(const std::string& filePath)
{
	file = filePath;
	std::ifstream file(filePath, std::ifstream::binary);
	if (!file) 
	{
		std::cout << __FUNCTION__": Fatal Error! Can't open shader file: " << filePath << "!" << std::endl;
		return;
	}

	char byte4Buffer[4] = {0};
	
	//Checking file type
	file.read(byte4Buffer, 4);
	if(strcmp(byte4Buffer, "301"))
	{
		std::cout << __FUNCTION__": Fatal Error! File " << filePath << " has unknown format!" << std::endl;
		return;
	}

	//Shader stages
	file.read(byte4Buffer, sizeof(unsigned int));
	unsigned int numberOfStages = 0;
	numberOfStages = *reinterpret_cast<unsigned int*>(byte4Buffer);

	std::vector<FileHeader> fileHeaders(numberOfStages);
	for (unsigned int i = 0; i < numberOfStages; i++) 
	{
		FileHeader header;
		file.read(byte4Buffer, sizeof(unsigned int));
		header.type = *reinterpret_cast<unsigned int*>(byte4Buffer);
		
		file.read(byte4Buffer, sizeof(unsigned int));
		header.size = *reinterpret_cast<unsigned int*>(byte4Buffer);
		
		file.read(byte4Buffer, sizeof(unsigned int));
		header.ptrInFile = *reinterpret_cast<unsigned int*>(byte4Buffer);
		fileHeaders[i] = header;
	}

	for (unsigned int i = 0; i < fileHeaders.size(); i++)
	{
		FileHeader header = fileHeaders[i];
		if (header.type == 1) 
		{
			hasVertexShader = true;
			vertexShaderSPIRV.resize(header.size / sizeof(unsigned int));
			file.seekg(header.ptrInFile);
			file.read(reinterpret_cast<char*>(vertexShaderSPIRV.data()), header.size);
		}

		else if (header.type == 2) 
		{ 
			hasFragmentShader = true;
			fragmentShaderSPIRV.resize(header.size / sizeof(unsigned int));
			file.seekg(header.ptrInFile);
			file.read(reinterpret_cast<char*>(fragmentShaderSPIRV.data()), header.size);
		}

	}

	file.close();
	
}
