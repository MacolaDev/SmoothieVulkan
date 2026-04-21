#include "Shader.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include "SmoothieCore.h"
#include <filesystem>
#include <mutex>
#include <cstring>

using namespace Smoothie;

int ShaderFile::serialize(std::ofstream& data){return 0;}


static int _read_std_string(std::ifstream &data, std::string& str)
{
	unsigned int _str_len = 0;
	if (!data.read(reinterpret_cast<std::ostream::char_type *>(&_str_len), sizeof(_str_len))) return 1;

	str.resize(_str_len);
	if (!data.read(str.data(), _str_len)) return 1;

	return 0;
}

static int _read_Variable(std::ifstream& data, Variable& variable)
{
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.location_offset_in), sizeof(variable.location_offset_in))) return 1;
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.location_offset_out), sizeof(variable.location_offset_out))) return 1;
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.uniform_offset), sizeof(variable.uniform_offset))) return 1;
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.bindings_offset), sizeof(variable.bindings_offset))) return 1;
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.flags), sizeof(variable.flags))) return 1;
	if (_read_std_string(data, variable.name) != 0) return 1;
	if (_read_std_string(data, variable.type) != 0) return 1;

	return 0;
}

static int _read_Variable_Global(std::ifstream& data, Variable_Global& variable)
{
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.set), sizeof(variable.set))) return 1;
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.binding), sizeof(variable.binding))) return 1;
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.stage), sizeof(variable.stage))) return 1;
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.flags), sizeof(variable.flags))) return 1;
	if (_read_std_string(data, variable.name) != 0) return 1;
	if (_read_std_string(data, variable.type) != 0) return 1;
	return 0;
}

int ShaderFile::de_serialize(std::ifstream& file)
{
	unsigned int _filePtr = 0;

	char _magic[4] = {0};
	if (!file.read(_magic, sizeof(_magic))) return 1;
	if (std::memcmp(_magic, s_Magic, sizeof(_magic)) != 0) return 1;

	unsigned int _spir_v_quads_size = 0;
	if (!file.read(reinterpret_cast<std::istream::char_type *>(&_spir_v_quads_size), sizeof(_spir_v_quads_size))) return 1;

	m_SPIR_V_CODE.resize(_spir_v_quads_size);
	if (!file.read(reinterpret_cast<std::istream::char_type *>(m_SPIR_V_CODE.data()), _spir_v_quads_size * sizeof(unsigned int))) return 1;

	unsigned int _entry_points_count = 0;
	if (!file.read(reinterpret_cast<std::istream::char_type *>(&_entry_points_count), sizeof(_entry_points_count))) return 1;

	m_EntryPoints.resize(_entry_points_count);
	for (unsigned int i = 0; i < _entry_points_count; i++)
	{
		auto& _entry_point = m_EntryPoints[i];
		if (!file.read(reinterpret_cast<std::istream::char_type *>(&_entry_point.flags), sizeof(_entry_point.flags))) return 1;
		if (!file.read(reinterpret_cast<std::istream::char_type *>(&_entry_point.stage), sizeof(_entry_point.stage))) return 1;
		if (_read_std_string(file, _entry_point.pipeline) != 0) return 1;
		if (_read_Variable(file, _entry_point.returnVariable) != 0) return 1;
		if (_read_std_string(file, _entry_point.name) != 0) return 1;

		unsigned int _inputVariables_count = 0;
		if (!file.read(reinterpret_cast<std::istream::char_type *>(&_inputVariables_count), sizeof(_inputVariables_count))) return 1;
		_entry_point.inputVariables.resize(_inputVariables_count);
		for (unsigned int k = 0; k < _inputVariables_count; k++)
		{
			if (_read_Variable(file, _entry_point.inputVariables[k]) != 0) return 1;
		}

	}


	unsigned int _global_variables_count = 0;
	if (!file.read(reinterpret_cast<std::istream::char_type *>(&_global_variables_count), sizeof(_global_variables_count))) return 1;
	m_GlobalVariables.resize(_global_variables_count);
	for (unsigned int i = 0; i < _global_variables_count; i++)
	{
		if (_read_Variable_Global(file, m_GlobalVariables[i]) != 0) return 1;
	}

	unsigned int _types_count = 0;
	m_Types.reserve(_types_count);
	if (!file.read(reinterpret_cast<std::istream::char_type *>(&_types_count), sizeof(_types_count))) return 1;
	for (int _index = 0; _index < _types_count; _index++)
	{

		Shader_Type_Kind _kind;
		std::string _name;
		if (!file.read(reinterpret_cast<std::istream::char_type *>(&_kind), sizeof(_kind))) return 1;
		if (_read_std_string(file, _name) != 0) return 1;

		switch (_kind)
		{
			case Shader_Type_Kind::Uniform:
			{
				auto _newType = std::make_shared<Shader_Type_Uniform>();
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->builtinType), sizeof(_newType->builtinType))) return 1;
				if (_read_std_string(file, _newType->userType) != 0) return 1;
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->subclass), sizeof(_newType->subclass))) return 1;
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->sizeX), sizeof(_newType->sizeX))) return 1;
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->sizeY), sizeof(_newType->sizeY))) return 1;
				m_Types.insert({std::move(_name), std::reinterpret_pointer_cast<Shader_Type_Base>(_newType)});

			}break;

			case Shader_Type_Kind::Resource:
			{
				auto _newType = std::make_shared<Shader_Type_Resource>();
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->flags), sizeof(_newType->flags))) return 1;
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->access), sizeof(_newType->access))) return 1;
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->shape), sizeof(_newType->shape))) return 1;
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->shape_flags), sizeof(_newType->shape_flags))) return 1;
				if (_read_std_string(file, _newType->result_type) != 0) return 1;
				m_Types.insert({std::move(_name), std::reinterpret_pointer_cast<Shader_Type_Base>(_newType)});

			}break;

			case Shader_Type_Kind::Struct:
			{
				auto _newType = std::make_shared<Shader_Type_Struct>();
				unsigned int _struct_member_count = 0;
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_struct_member_count), sizeof(_struct_member_count))) return 1;
				_newType->members.resize(_struct_member_count);
				for (unsigned int k = 0; k < _struct_member_count; k++)
				{
					if (_read_Variable(file, _newType->members[k]) != 0) return 1;
				}

				m_Types.insert({std::move(_name), std::reinterpret_pointer_cast<Shader_Type_Base>(_newType)});
			}break;


			default:
				break;
		}

	}


	return 0;
}

int Smoothie::ShaderFile::create(const std::string& shaderFile)
{
	m_Filepath = shaderFile;
	if (!std::filesystem::exists(m_Filepath))
	{
		std::cout << "No file named: " << m_Filepath << std::endl;
		return 1;
	}

	const auto file_size = std::filesystem::file_size(m_Filepath);
	std::vector<char> binary_data(file_size);
	auto file = std::ifstream(m_Filepath, std::ios_base::binary);

	if (de_serialize(file) != 0)
	{
		std::cout << "Failed to parse file: " << m_Filepath << std::endl;
		file.close();
		return 1;
	}
	file.close();

	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = m_SPIR_V_CODE.size() * sizeof(uint32_t);
	assert(shaderModuleCreateInfo.codeSize != 0);
	shaderModuleCreateInfo.pCode = m_SPIR_V_CODE.data();
	assert(shaderModuleCreateInfo.pCode != nullptr);
	shaderModuleCreateInfo.pNext = nullptr;
	shaderModuleCreateInfo.flags = 0;

	if (
		!static_cast<int>(m_Flags ^ ShaderFile_CreateFlags::DontCreateModule) &&
		vkCreateShaderModule(SmoothieCore::getDevice(), &shaderModuleCreateInfo, nullptr, &m_ShaderModule) != VK_SUCCESS)
	{
		std::cout << "Failed to create system shader module!" << std::endl;
		return 1;
	}

	if (!static_cast<int>(m_Flags ^ ShaderFile_CreateFlags::Hold_SPIR_V))
	{
		m_SPIR_V_CODE.clear();
	}

	return 0;
}

void Smoothie::ShaderFile::destroy()
{
	if (m_ShaderModule != nullptr)
	{
		vkDestroyShaderModule(SmoothieCore::getDevice(), m_ShaderModule, nullptr);
		m_ShaderModule = nullptr;
	}
}

