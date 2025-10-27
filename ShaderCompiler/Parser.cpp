#include "Parser.h"
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
using namespace glslang;

static void getIncludeDepthAndFileName(const std::string& headerFile, unsigned int& depth, std::string& fileNameAndFolder)
{
	size_t includeDepth = 0;
	size_t fileNameAndFolderIndex = 0;
	for (size_t i = 0; i < headerFile.size() / (3 * sizeof(char)); i++)
	{
		const char dotDotSlash[3] =
		{
			headerFile[3 * i], headerFile[3 * i + 1],  headerFile[3 * i + 2]
		};

		if ((dotDotSlash[0] == '.') && (dotDotSlash[1] == '.') && (dotDotSlash[2] == '/')) 
		{
			includeDepth++;
		}
		else
		{
			fileNameAndFolderIndex = 3 * i;
			break;
		}

	}
	
	depth = static_cast<unsigned int>(includeDepth);

	fileNameAndFolder.resize(headerFile.size() - fileNameAndFolderIndex);
	for (size_t i = 0; i <  headerFile.size() - fileNameAndFolderIndex; i++)
	{
		fileNameAndFolder[i] = headerFile[i + fileNameAndFolderIndex];
	}
}

static std::string getRootFolderOfIncludedFile(unsigned int inclusionDepth)
{
	const auto sourceFilePath = std::filesystem::path(InputArguments::sourceFile);
	if (!sourceFilePath.has_parent_path()) return "";

	const auto sourceFileDir = sourceFilePath.parent_path();
	std::filesystem::path folder = sourceFileDir;
	for (unsigned int i = 0; i < inclusionDepth; i++)
	{
		if (folder.has_parent_path())
		{
			folder = folder.parent_path();
		}
		else
		{
			return "";
		}
	}
	return folder.string();
}

static void readDataFromFile(std::string& dataFromFile, const std::string& filepath) 
{
	std::ifstream file(filepath, std::ios::binary);
	auto fileSize = std::filesystem::file_size(filepath);
	dataFromFile.resize(fileSize, '\0');
	file.read(dataFromFile.data(), fileSize);
	file.close();
}

TShader::Includer::IncludeResult* SmoothieIncluder::includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth)
{
	std::string fileName;
	unsigned int includeDepth = 0;
	getIncludeDepthAndFileName(headerName, includeDepth,fileName);
	if ("" == fileName) return nullptr;

	std::string includeFileFolder = getRootFolderOfIncludedFile(includeDepth);
	if ("" == includeFileFolder) return nullptr;


	headerFileName = includeFileFolder + "/" + fileName;
	if (std::filesystem::exists(headerFileName))
	{
		readDataFromFile(dataFromFile, headerFileName);

		return new IncludeResult(headerFileName, dataFromFile.c_str(), dataFromFile.size(), nullptr);;
	}
	return nullptr;
}

TShader::Includer::IncludeResult* SmoothieIncluder::includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth)
{

	std::string fileName;
	unsigned int includeDepth = 0;
	getIncludeDepthAndFileName(headerName, includeDepth,fileName);
	if ("" == fileName) return nullptr;

	std::string includeFileFolder = getRootFolderOfIncludedFile(includeDepth);
	if ("" == includeFileFolder) return nullptr;
	
	headerFileName = includeFileFolder + "/" + fileName;
	if (std::filesystem::exists(headerFileName))
	{
		readDataFromFile(dataFromFile, headerFileName);

		return new IncludeResult(headerFileName, dataFromFile.c_str(), dataFromFile.size(), nullptr);;
	}
	return nullptr;
}

void SmoothieIncluder::releaseInclude(IncludeResult* result)
{
	delete result;
}
