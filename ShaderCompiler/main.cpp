#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "Parser.h"
#include <fstream>
#include <regex>

#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/Public/ResourceLimits.h"

const char executableDescription[] =
"\nCompiles GLSL source code into binary SPIR-V suitable for SmoothieVulkan render engine.\n\n";

const char validUsageMessage[] =
"Valid usage:\n"
"    ShaderCompiler --source-file [file] --output-file [file] []\n\n"
"Options:\n"
"    --source-file [file]		Path to GLSL source file to compile\n"
"    --output-file [file]		Path to output file\n\n"
"    -S                         Generate debugging symbols\n"
"\n";


static void infoMessage() 
{
	std::cout << executableDescription << validUsageMessage << std::endl;
	exit(0);
}

static void validUsage() 
{
	std::cout << validUsageMessage << std::endl;
	exit(0);
}

std::string InputArguments::compilerFile;
std::string InputArguments::sourceFile;
std::string InputArguments::destFile;

static void processInput(int argc, char* argv[]) 
{
	if (argc == 1) infoMessage();

	InputArguments::compilerFile = argv[0];
	
	if (!strcmp(argv[1], "--source-file")) 
	{
		if (!std::filesystem::exists(argv[2])) 
		{
			std::cout << "Invalid arguments or invalid path to --source-file: " << argv[2] << std::endl;
			validUsage();
		}
		else
		{
			InputArguments::sourceFile = argv[2];
		}

	}
	else
	{
		validUsage();
	}

	if (!strcmp(argv[3], "--output-file"))
	{
		InputArguments::destFile = argv[4];
	}
	else
	{
		validUsage();
	}

	//Other arguments
	for (int i = 5; i < argc; i++) 
	{
		
	}
}

static void readFile(std::string& sourceCode)
{
	std::ifstream file(InputArguments::sourceFile, std::ios::binary);
	if (!file) 
	{
		std::cout << "Invalid input file!" << std::endl;
		validUsage();
	}
	auto fileSize = std::filesystem::file_size(InputArguments::sourceFile);

	sourceCode.resize(fileSize, '\0');
	file.read(sourceCode.data(), fileSize);
	file.close();
}

struct ShaderStage 
{
	EShLanguage shaderStage;
	std::string code;
};

static void getShaderStages(const std::string& sourceCode, std::vector<ShaderStage>& shaderStages)
{
	std::regex pattern("#ifdef\\b[\\s\\S]*?#endif");
	
	auto words_begin = std::sregex_iterator(sourceCode.begin(), sourceCode.end(), pattern);
	auto words_end = std::sregex_iterator();

	for (std::sregex_iterator it = words_begin; it != words_end; it++) 
	{
		std::string shader = it->str();
		std::regex stagePattern("[A-Z]+_SHADER");
		std::smatch matches;
		if (!std::regex_search(shader, matches, stagePattern)) 
		{
			std::cout << "Valid syntax for source code must be:\n" << "  #ifdef [SHADER]_SHADER\n" << "  //Source code\n" << "  #endif" << std::endl;
			exit(0);
		}

		EShLanguage stage;
		if (matches[0].str() == "VERTEX_SHADER") 
		{
			shader = "#define VERTEX_SHADER\n" + shader;
			stage = EShLangVertex;
		}
		else if (matches[0].str() == "FRAGMENT_SHADER")
		{
			shader = "#define FRAGMENT_SHADER\n" + shader;
			stage = EShLangFragment;
		}
		else
		{
			std::cout << "Failed to detect shader stage!" << std::endl;
			std::cout << "Source code that failed:\n\n" << shader << std::endl;
			stage = EShLangVertex;
			exit(0);
		}
		shaderStages.push_back({ stage, shader });
	}
}

static std::vector<unsigned int> compileShaders(const ShaderStage& shaderData)
{
	glslang::TShader* shader = new glslang::TShader(shaderData.shaderStage);

	const char* const code = shaderData.code.c_str();
	shader->setStrings(&code, 1);

	
	shader->setEnvInput(glslang::EShSourceGlsl, shaderData.shaderStage, glslang::EShClientVulkan, 100);
	shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
	shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
	shader->setDebugInfo(true);

	SmoothieIncluder includer;
	const TBuiltInResource* resources = GetDefaultResources();
	if (!shader->parse(resources, 450, ENoProfile, false, false, EShMsgDefault)) 
	{
		std::cout << "\nFailed to compile shader: " << InputArguments::sourceFile << std::endl;
		printf("\nGLSL Parsing Failed: %s\n", shader->getInfoLog());
		
		std::cout << "Code that failed: \n\n" << code << "\n" << std::endl;
		exit(0);
	}

	glslang::TProgram program; 
	program.addShader(shader);
	if (!program.link(EShMsgDefault)) 
	{
		printf("GLSL Linking Failed: %s\n", program.getInfoLog());
		exit(0);
	}
	glslang::SpvOptions spvOptions;
	spvOptions.generateDebugInfo = true;
	spvOptions.emitNonSemanticShaderDebugInfo = true;
	spvOptions.emitNonSemanticShaderDebugSource = true;
	spvOptions.disableOptimizer = true;

	std::vector<unsigned int> spirv;
	glslang::GlslangToSpv(*program.getIntermediate(shaderData.shaderStage), spirv, &spvOptions);
	delete shader;
	return spirv;
}

const char SHADER_FILE_HEADER[4] = "301";

struct FileHeader 
{
	unsigned int type; 
	unsigned int size; 
	unsigned int ptrInFile;
};


int main(int argc, char* argv[]) 
{
	processInput(argc, argv);

	std::string sourceCode;
	readFile(sourceCode);

	std::vector<ShaderStage> shaderStages;
	getShaderStages(sourceCode, shaderStages);

	glslang::InitializeProcess();
	glslang::FinalizeProcess();
	glslang::InitializeProcess();


	std::vector<std::vector<unsigned int>> SPIRVdata;
	for (int i = 0; i < shaderStages.size(); i++) 
	{
		SPIRVdata.push_back(compileShaders(shaderStages[i]));
	}
	glslang::FinalizeProcess();

	unsigned int headerDataOffset = static_cast<unsigned int>(sizeof(SHADER_FILE_HEADER) + sizeof(unsigned int) + sizeof(FileHeader) * shaderStages.size());

	std::ofstream outFile(InputArguments::destFile, std::ios::binary);

	outFile.write(SHADER_FILE_HEADER, sizeof(SHADER_FILE_HEADER)); //Magic number

	//Number of shader headers
	const unsigned int shaderStagesSize = static_cast<unsigned int>(shaderStages.size());
	outFile.write(reinterpret_cast<const char*>(&shaderStagesSize), sizeof(unsigned int));

	//Shader header data
	unsigned int fileOffset = headerDataOffset;
	for (unsigned int i = 0; i < shaderStages.size(); i++) 
	{
		FileHeader header;
		header.size = sizeof(unsigned int) * static_cast<unsigned int>(SPIRVdata[i].size());
		if (shaderStages[i].shaderStage == EShLangVertex) 
		{
			header.type = 1;
		}
		else if (shaderStages[i].shaderStage == EShLangFragment)
		{
			header.type = 2;
		}
		else
		{
			header.type = 0;
		}
		header.ptrInFile = fileOffset;

		outFile.write(reinterpret_cast<const char*>(&header.type), sizeof(unsigned int));
		outFile.write(reinterpret_cast<const char*>(&header.size), sizeof(unsigned int));
		outFile.write(reinterpret_cast<const char*>(&header.ptrInFile), sizeof(unsigned int));
		fileOffset += header.size;
	}

	//Spirv data
	for (unsigned int i = 0; i < SPIRVdata.size(); i++)
	{
		outFile.write(reinterpret_cast<const char*>(SPIRVdata[i].data()), sizeof(unsigned int) * SPIRVdata[i].size());
	}

	outFile.close();
}