#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "Parser.h"


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
		if (!std::filesystem::exists(argv[4]))
		{
			std::cout << "Invalid arguments or invalid path to --output-file: " << argv[4] << std::endl;
			validUsage();
		}
		else
		{
			InputArguments::destFile = argv[4];
		}

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


int main(int argc, char* argv[]) 
{
	processInput(argc, argv);
}