#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "Parser.h"
#include <fstream>
#include <regex>
#include <algorithm>

#include <vulkan/vulkan.h>

#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/Public/ResourceLimits.h"

#include "Core/Constants.h"
#include "ResourceManager/ShaderFile.h"
#include "glslang/glslang/Include/Types.h"

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

struct ShaderCompilerData
{
	std::string compilation_preamble;
	_ShaderModule_file_data data;
	EShLanguage eshLanguage = EShLangCount;
};

#define GET_VULAKN_STAGE_BIT(stage) VK_SHADER_STAGE_##stage##_BIT
#define GET_LANG_STAGE_BIT(stage) EShLang##stage

static void getShaderData(const std::string& sourceCode, std::vector<ShaderCompilerData>& data)
{
	std::regex pattern("#ifdef\\s+SSHADER_(VERTEX|TESCONT|TESEVAL|GEOMETRY|FRAGMENT|COMPUTE)_([a-zA-Z0-9_]+)");
	
	auto words_begin = std::sregex_iterator(sourceCode.begin(), sourceCode.end(), pattern);
	auto words_end = std::sregex_iterator();

	for (std::sregex_iterator it = words_begin; it != words_end; it++) 
	{
		ShaderCompilerData compiler_data;
		std::smatch match = *it;
		std::string stage = match[1];
		std::string pipline_name = match[2];

		compiler_data.data.shader_name = pipline_name;
		compiler_data.data.shader_name_size = static_cast<unsigned int>(pipline_name.size());

		compiler_data.compilation_preamble = "\n#define SSHADER_" + stage + "_" + pipline_name + "\n";
		if (stage == "VERTEX")
		{
			compiler_data.eshLanguage = GET_LANG_STAGE_BIT(Vertex);
			compiler_data.data.shader_stage = GET_VULAKN_STAGE_BIT(VERTEX);
		}
		else if(stage == "TESCONT")
		{
			compiler_data.eshLanguage = GET_LANG_STAGE_BIT(TessControl);
			compiler_data.data.shader_stage = GET_VULAKN_STAGE_BIT(TESSELLATION_CONTROL);
		}
		else if (stage == "TESEVAL")
		{
			compiler_data.eshLanguage = GET_LANG_STAGE_BIT(TessEvaluation);
			compiler_data.data.shader_stage = GET_VULAKN_STAGE_BIT(TESSELLATION_EVALUATION);
		}
		else if (stage == "GEOMETRY")
		{
			compiler_data.eshLanguage = GET_LANG_STAGE_BIT(Geometry);
			compiler_data.data.shader_stage = GET_VULAKN_STAGE_BIT(GEOMETRY);
		}
		else if (stage == "FRAGMENT")
		{
			compiler_data.eshLanguage = GET_LANG_STAGE_BIT(Fragment);
			compiler_data.data.shader_stage = GET_VULAKN_STAGE_BIT(FRAGMENT);
		}
		else if (stage == "COMPUTE")
		{
			compiler_data.eshLanguage = GET_LANG_STAGE_BIT(Compute);
			compiler_data.data.shader_stage = GET_VULAKN_STAGE_BIT(COMPUTE);
		}
		else
		{
			std::cout << "Can't compile shader: " << it->str() << "\nNo suppoerd shader named: " << stage << std::endl;
			exit(0);
		}
		data.push_back(compiler_data);
	}
}

static ModelDescriptorDataType getDataBasedOnGlslAngType(const glslang::TType* type)
{
	if (type == nullptr) return MODEL_DESCRIPTOR_DATA_TYPE_NO_TYPE;
	if ((type->getBasicType() == glslang::EbtBlock) && (type->getQualifier().storage == glslang::EvqUniform))
	{
		if (type->getQualifier().storage == glslang::EvqUniform)
		{
			return MODEL_DESCRIPTOR_UNIFORM_BLOCK;
		}
		if (type->getQualifier().storage == glslang::EvqBuffer)
		{
			return MODEL_DESCRIPTOR_STORAGE_BLOCK;
		}
	}

	return MODEL_DESCRIPTOR_DATA_TYPE_NO_TYPE;
}

struct DescriptorSetFinder
{
	int set = -1;
	int binding = -1;
	std::vector<std::string> descriptor_set_variable_names;
};

using DescriptorSetMap = std::unordered_map<std::string, DescriptorSetFinder>;

static void __find_variables_in_sets(const DescriptorSetMap& __sets, const std::string& __variable_name, int& set, int& binding)
{
	for (const auto& [__set_name, __set] : __sets)
	{
		for (const auto& __set_variable : __set.descriptor_set_variable_names)
		{
			if (__set_variable == __variable_name)
			{
				set = __set.set;
				binding = __set.binding;
				return;
			}

		}
	}
}

static void getFileReflectionFromlang(const glslang::TObjectReflection& reflection, unsigned int stage, _Reflection_type_file_data type, _ShaderDescriptorsReflection_file_data& result, DescriptorSetMap& descriptor_block_data)
{
	result.name = reflection.name;
	result.name_size = reflection.name.size();
	result.offset = reflection.offset;
	result.data_type = (reflection.glDefineType == -1 ) ? getDataBasedOnGlslAngType(reflection.getType()) : static_cast<ModelDescriptorDataType>(reflection.glDefineType);
	result.size = reflection.size;
	result.index = reflection.index;
	result.counterIndex = reflection.index;
	result.numMembers = reflection.numMembers;
	result.arrayStride = reflection.arrayStride;
	result.topLevelArraySize = reflection.topLevelArraySize;
	result.topLevelArrayStride = reflection.topLevelArrayStride;
	result.reflection_type = type;
	result.stages = stage;
	
	result.location = -1;
	result.binding = -1;
	result.set = -1;

	result.owning_set = -1;
	result.owning_set_binding = -1;

	const glslang::TType* __glslang_type = reflection.getType();
	if (__glslang_type == nullptr) return;

	
	const auto& qualifier = __glslang_type->getQualifier();
	result.location = (qualifier.hasLocation() == true) ? qualifier.layoutLocation : -1;
	result.binding = (qualifier.hasBinding() == true) ? qualifier.layoutBinding : -1;
	result.set = (qualifier.hasSet() == true) ? qualifier.layoutSet : -1;

	//If type is a struct, read its contents and store it
	if (__glslang_type->isStruct())
	{
		const auto* __owning_struct = __glslang_type->getStruct();
		if (__owning_struct != nullptr)
		{

			auto& __current_set = descriptor_block_data[reflection.name];
			__current_set.binding = result.binding;
			__current_set.set = result.set;
			for (const auto& __child : *__owning_struct)
			{
				__current_set.descriptor_set_variable_names.push_back(__child.type->getFieldName().c_str());
			}
			int a = 0;
		}
	}

	if ((result.binding < 0) && (result.set < 0) && (result.location < 0))
	{
		const auto& __variable_name = result.name;
		
		std::vector<int> __dot_pos;
		for (int i = 0; i < __variable_name.size(); i++)
		{
			if (__variable_name[i] == '.')
			{
				__dot_pos.push_back(i); 
			}
		}
		
		if (__dot_pos.empty()) //global variables dont have any prefix, so we search all sets for that variable name.
		{
			__find_variables_in_sets(descriptor_block_data, __variable_name, result.owning_set, result.owning_set_binding);
		}
		else
		{
			const auto& __set_name = __variable_name.substr(0, __dot_pos[0]);
			if (descriptor_block_data.find(__set_name) != descriptor_block_data.end())
			{
				//That set exists, binding is equal to that set
				const auto& __set = descriptor_block_data.at(__set_name);
				result.owning_set = __set.set;
				result.owning_set_binding = __set.binding;
			}
			else //This is the global variable aswell 
			{
				__find_variables_in_sets(descriptor_block_data, __set_name, result.owning_set, result.owning_set_binding);
			}
		}
	}

	result._block_size = result.get_object_size();
}

static void compileShaders(const std::string& source, std::vector<ShaderCompilerData>& data)
{
	for (size_t i = 0; i < data.size(); i++)
	{
		auto& shader_data = data[i].data;

		glslang::TShader shader = glslang::TShader(data[i].eshLanguage);
		const char* const code = source.c_str();
		shader.setStrings(&code, 1);

		//Preambles
		shader.setPreamble(data[i].compilation_preamble.c_str());
		
		//Enviroment
		shader.setEnvInput(glslang::EShSourceGlsl, data[i].eshLanguage, glslang::EShClientVulkan, 100);
		shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
		shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
		shader.setDebugInfo(true);
		
		//Source files for debugging
		shader.setSourceFile(InputArguments::sourceFile.c_str());
		shader.addSourceText(code, std::strlen(code));

		//Compile shaders
		SmoothieIncluder includer;
		const TBuiltInResource* resources = GetDefaultResources();
		if (!shader.parse(resources, 450, ENoProfile, false, false, EShMsgDefault, includer))
		{
			std::cout << "\nFailed to compile shader: " << InputArguments::sourceFile << std::endl;
			printf("\nGLSL Parsing Failed: %s\n", shader.getInfoLog());
			exit(0);
		}

		//Link shaders
		glslang::TProgram program; 
		program.addShader(&shader);
		if (!program.link(EShMsgDefault)) 
		{
			printf("GLSL Linking Failed: %s\n", program.getInfoLog());
			exit(0);
		}

		//SPIRV from intermidiate
		glslang::SpvOptions spvOptions;
		spvOptions.stripDebugInfo = true;
		spvOptions.generateDebugInfo = false;
		spvOptions.emitNonSemanticShaderDebugInfo = false;
		spvOptions.emitNonSemanticShaderDebugSource = false;
		spvOptions.disableOptimizer = false;
		spv::SpvBuildLogger logger;
		glslang::GlslangToSpv(*program.getIntermediate(data[i].eshLanguage), shader_data.SPIRV_data, &logger, &spvOptions);
		shader_data.SPIRV_data_size = shader_data.SPIRV_data.size() * sizeof(unsigned int);


		//Build reflections
		if (!program.buildReflection(EShReflectionDefault | EShReflectionAllIOVariables | EShReflectionUnwrapIOBlocks)) continue;
		

		for (int index = 0; index < program.getNumPipeInputs(); index++)
		{
			_ShaderDescriptorsReflection_file_data _file_data;
			DescriptorSetMap _dummy;
			getFileReflectionFromlang(program.getPipeInput(index), shader_data.shader_stage, _Reflection_type_file_data::PIPE_INPUT, _file_data, _dummy);
			shader_data.reflections.push_back(_file_data);
		}

		for (int index = 0; index < program.getNumPipeOutputs(); index++)
		{
			_ShaderDescriptorsReflection_file_data _file_data;
			DescriptorSetMap _dummy;
			getFileReflectionFromlang(program.getPipeOutput(index), shader_data.shader_stage, _Reflection_type_file_data::PIPE_OUTPUT, _file_data, _dummy);
			shader_data.reflections.push_back(_file_data);
		}

		
		DescriptorSetMap __maps;

		for (int index = 0; index < program.getNumUniformBlocks(); index++)
		{
			_ShaderDescriptorsReflection_file_data _file_data;
			getFileReflectionFromlang(program.getUniformBlock(index), shader_data.shader_stage, _Reflection_type_file_data::UNIFORM_BLOCK, _file_data, __maps);
			shader_data.reflections.push_back(_file_data);
		}

		for (int index = 0; index < program.getNumBufferBlocks(); index++)
		{
			_ShaderDescriptorsReflection_file_data _file_data;
			getFileReflectionFromlang(program.getBufferBlock(index), shader_data.shader_stage, _Reflection_type_file_data::STORAGE_BLOCK, _file_data, __maps);
			shader_data.reflections.push_back(_file_data);
		}

		for (int index = 0; index < program.getNumUniformVariables(); index++)
		{
			_ShaderDescriptorsReflection_file_data _file_data;
			getFileReflectionFromlang(program.getUniform(index), shader_data.shader_stage, _Reflection_type_file_data::UNIFORM_VARIABLE, _file_data, __maps);
			shader_data.reflections.push_back(_file_data);
		}

		for (int index = 0; index < program.getNumBufferVariables(); index++)
		{
			_ShaderDescriptorsReflection_file_data _file_data;
			getFileReflectionFromlang(program.getBufferVariable(index), shader_data.shader_stage, _Reflection_type_file_data::STORAGE_VARIABLE, _file_data, __maps);
			shader_data.reflections.push_back(_file_data);
		}
	
		shader_data.reflections_count = static_cast<unsigned int>(shader_data.reflections.size());
		for (size_t i = 0; i < shader_data.reflections.size(); i++)
		{
			shader_data.reflections_byte_size += shader_data.reflections[i].get_object_size();
		}

		shader_data._block_size = shader_data.get_object_size();
	}
}

int main(int argc, char* argv[]) 
{
	processInput(argc, argv);

	std::string sourceCode;
	readFile(sourceCode);

	std::vector<ShaderCompilerData> shader_data;
	getShaderData(sourceCode, shader_data);

	glslang::InitializeProcess();
	glslang::FinalizeProcess();
	glslang::InitializeProcess();
	
	compileShaders(sourceCode, shader_data);

	glslang::FinalizeProcess();

	_ShaderFile_file_data file_data;
	file_data.shaders_count = static_cast<unsigned int>(shader_data.size());
	for (size_t i = 0; i < shader_data.size(); i++)
	{
		file_data.shaders_byte_size += shader_data[i].data.get_object_size();
		file_data.shaders.push_back(shader_data[i].data);
	}

	std::vector<char> file_data_binary;
	file_data.write_to_buffer(file_data_binary);

	std::ofstream outFile(InputArguments::destFile, std::ios::binary);
	outFile.write(file_data_binary.data(), file_data_binary.size());
	outFile.close();
}