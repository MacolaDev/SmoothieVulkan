#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <tuple>
#include "ResourceManager/ShaderFile.h"
#include "Smoothie.h"

namespace SmoothieExporter
{
	struct ShaderPipeInput
	{
		unsigned int location = -1;
		ModelDescriptorDataType type;
		std::string name;
	};

	struct ShaderVariable 
	{
		unsigned int set = -1;
		unsigned int binding = -1;
		ModelDescriptorDataType type;
		std::string name;
	};

	struct ShaderStage
	{
		std::vector<ShaderPipeInput> pipeInputs; //As of now, this is only used in vertex shader.
		std::vector<ShaderVariable> pipeVariables;
	};

	using ShaderKey = Smoothie::ShaderFile::SearchKey;
	using ShaderKeyHash = Smoothie::ShaderFile::SearchKeyHash;
	using ShaderFileMap = std::unordered_map<ShaderKey, ShaderStage, ShaderKeyHash>;
	int parse_shader_file(const std::string& filepath, ShaderFileMap& stages);
}
