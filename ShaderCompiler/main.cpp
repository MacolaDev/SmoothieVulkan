#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <fstream>

#include "Compiler.h"


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


static void processInput(int argc, char* argv[], SmoothieCompiler::Settings& settings)
{
	if (argc == 1) infoMessage();

	settings.compiler_file = argv[0];
	
	if (!std::strcmp(argv[1], "--source-file"))
	{
		if (!std::filesystem::exists(argv[2])) 
		{
			std::cout << "Invalid arguments or invalid path to --source-file: " << argv[2] << std::endl;
			validUsage();
		}
		else
		{
			settings.input_file = argv[2];
		}

	}
	else
	{
		validUsage();
	}

	if (!strcmp(argv[3], "--output-file"))
	{
		settings.output_file = argv[4];
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


using namespace SmoothieCompiler;

int main(int argc, char* argv[]) 
{
	Settings _settings;
	processInput(argc, argv, _settings);
	CompiledShader _compiledShader;
	_compiledShader.m_CompilerSettings = _settings;
	if (_compiledShader.compile_shader() != 0)
	{
		std::cout << "Error showed during compilation!" << std::endl;
		return 0;
	}

	std::ofstream _out(_settings.output_file, std::ios::binary);
	_compiledShader.serialize(_out);
	_out.close();
	std::cout << "File written to: " << _settings.output_file << std::endl;
	return 0;
}
