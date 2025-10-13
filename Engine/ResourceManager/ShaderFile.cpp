#include "ShaderFile.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <memory>

unsigned int _ShaderDescriptorsReflection_file_data::get_object_size() const
{
	auto __buffer_size = 0;
	__buffer_size += sizeof(_block_size);
	__buffer_size += sizeof(offset);
	__buffer_size += sizeof(data_type);
	__buffer_size += sizeof(reflection_type);
	__buffer_size += sizeof(size);
	__buffer_size += sizeof(index);
	__buffer_size += sizeof(counterIndex);
	__buffer_size += sizeof(numMembers);
	__buffer_size += sizeof(arrayStride);
	__buffer_size += sizeof(topLevelArraySize);
	__buffer_size += sizeof(topLevelArrayStride);
	__buffer_size += sizeof(stages);
	__buffer_size += sizeof(location);
	__buffer_size += sizeof(binding);
	__buffer_size += sizeof(set);
	__buffer_size += sizeof(owning_set);
	__buffer_size += sizeof(owning_set_binding);

	__buffer_size += sizeof(name_size);
	__buffer_size += static_cast<unsigned int>(name.size());
	return __buffer_size;
}

#define memccpy_type_to_buffer(_buffer_offset, data)\
		std::memcpy(&buffer[_buffer_offset], &data, sizeof(data));\
		_buffer_offset += sizeof(data); \

void _ShaderDescriptorsReflection_file_data::write_to_buffer(std::vector<char>& buffer) const
{
	buffer.resize(get_object_size());
	unsigned int buffer_offset = 0;
	memccpy_type_to_buffer(buffer_offset, _block_size);
	memccpy_type_to_buffer(buffer_offset, offset);
	memccpy_type_to_buffer(buffer_offset, data_type);
	memccpy_type_to_buffer(buffer_offset, reflection_type);
	memccpy_type_to_buffer(buffer_offset, size);
	memccpy_type_to_buffer(buffer_offset, index);
	memccpy_type_to_buffer(buffer_offset, counterIndex);
	memccpy_type_to_buffer(buffer_offset, numMembers);
	memccpy_type_to_buffer(buffer_offset, arrayStride);
	memccpy_type_to_buffer(buffer_offset, topLevelArraySize);
	memccpy_type_to_buffer(buffer_offset, topLevelArrayStride);
	memccpy_type_to_buffer(buffer_offset, stages);

	memccpy_type_to_buffer(buffer_offset, location);
	memccpy_type_to_buffer(buffer_offset, binding);
	memccpy_type_to_buffer(buffer_offset, set);

	memccpy_type_to_buffer(buffer_offset, owning_set);
	memccpy_type_to_buffer(buffer_offset, owning_set_binding);

	memccpy_type_to_buffer(buffer_offset, name_size);

	std::memcpy(&buffer[buffer_offset], &name[0], name.size());
	buffer_offset += name.size();

}

#define memccpy_buffer_to_type(_buffer_offset, data)\
std::memcpy(&data, &buffer[_buffer_offset], sizeof(data));\
_buffer_offset += sizeof(data);\

int _ShaderDescriptorsReflection_file_data::get_object_from_buffer(const std::vector<char>& buffer)
{
	if (buffer.size() < sizeof(_block_size)) return 1;
	unsigned int __buffer_offset = 0;
	memccpy_buffer_to_type(__buffer_offset, _block_size);
	if (_block_size != buffer.size()) return 1;

	memccpy_buffer_to_type(__buffer_offset, offset);
	memccpy_buffer_to_type(__buffer_offset, data_type);
	memccpy_buffer_to_type(__buffer_offset, reflection_type);
	memccpy_buffer_to_type(__buffer_offset, size);
	memccpy_buffer_to_type(__buffer_offset, index);
	memccpy_buffer_to_type(__buffer_offset, counterIndex);
	memccpy_buffer_to_type(__buffer_offset, numMembers);
	memccpy_buffer_to_type(__buffer_offset, arrayStride);
	memccpy_buffer_to_type(__buffer_offset, topLevelArraySize);
	memccpy_buffer_to_type(__buffer_offset, topLevelArrayStride);
	memccpy_buffer_to_type(__buffer_offset, stages);

	memccpy_buffer_to_type(__buffer_offset, location);
	memccpy_buffer_to_type(__buffer_offset, binding);
	memccpy_buffer_to_type(__buffer_offset, set);

	memccpy_buffer_to_type(__buffer_offset, owning_set);
	memccpy_buffer_to_type(__buffer_offset, owning_set_binding);

	memccpy_buffer_to_type(__buffer_offset, name_size);
	name.resize(name_size);
	std::memcpy(&name[0], &buffer[__buffer_offset], name_size);
	__buffer_offset += name_size;
	return 0;
}

unsigned int _ShaderModule_file_data::get_object_size() const
{
	unsigned int __buffer_size = 0;
	__buffer_size += sizeof(_block_size);
	__buffer_size += sizeof(shader_name_size);
	__buffer_size += static_cast<unsigned int>(shader_name.size());

	__buffer_size += sizeof(shader_stage);

	__buffer_size += sizeof(reflections_count);
	__buffer_size += sizeof(reflections_byte_size);
	for (size_t i = 0; i < reflections.size(); i++)
	{
		__buffer_size += reflections[i].get_object_size();
	}
	

	__buffer_size += sizeof(SPIRV_data_size);
	__buffer_size += sizeof(unsigned int) * static_cast<unsigned int>(SPIRV_data.size());
	return __buffer_size;
}

void _ShaderModule_file_data::write_to_buffer(std::vector<char>& buffer) const
{
	buffer.resize(get_object_size());
	unsigned int buffer_offset = 0;
	memccpy_type_to_buffer(buffer_offset, _block_size);
	memccpy_type_to_buffer(buffer_offset, shader_name_size);

	std::memcpy(&buffer[buffer_offset], shader_name.c_str(), shader_name.size());
	buffer_offset += shader_name.size();

	memccpy_type_to_buffer(buffer_offset, shader_stage);

	memccpy_type_to_buffer(buffer_offset, reflections_count);
	memccpy_type_to_buffer(buffer_offset, reflections_byte_size);
	for (size_t i = 0; i < reflections.size(); i++)
	{
		std::vector<char> __subbuffer_data;
		reflections[i].write_to_buffer(__subbuffer_data);
		std::memcpy(&buffer[buffer_offset], __subbuffer_data.data(), __subbuffer_data.size());
		buffer_offset += static_cast<unsigned int>(__subbuffer_data.size());
	}

	memccpy_type_to_buffer(buffer_offset, SPIRV_data_size);

	std::memcpy(&buffer[buffer_offset], SPIRV_data.data(), static_cast<unsigned int>(SPIRV_data.size()) * sizeof(unsigned int));
	buffer_offset += static_cast<unsigned int>(SPIRV_data.size()) + sizeof(unsigned int);

}

int _ShaderModule_file_data::get_object_from_buffer(const std::vector<char>& buffer)
{
	if (buffer.size() < sizeof(_block_size)) return 1;
	unsigned int __buffer_offset = 0;
	memccpy_buffer_to_type(__buffer_offset, _block_size);
	if (_block_size != buffer.size()) return 1;

	memccpy_buffer_to_type(__buffer_offset, shader_name_size);

	shader_name.resize(shader_name_size);
	std::memcpy(&shader_name[0], &buffer[__buffer_offset], shader_name_size);
	__buffer_offset += shader_name_size;

	memccpy_buffer_to_type(__buffer_offset, shader_stage);
	memccpy_buffer_to_type(__buffer_offset, reflections_count);
	reflections.resize(reflections_count);

	memccpy_buffer_to_type(__buffer_offset, reflections_byte_size);

	for (size_t i = 0; i < reflections_count; i++)
	{
		unsigned int next_block_size = 0;
		std::memcpy(&next_block_size, &buffer[__buffer_offset], sizeof(next_block_size));
		
		std::vector<char> _subbuffer(next_block_size);
		std::memcpy(&_subbuffer[0], &buffer[__buffer_offset], next_block_size);

		_ShaderDescriptorsReflection_file_data _subdata;
		if(_subdata.get_object_from_buffer(_subbuffer) != 0) return 1;
		reflections[i] = _subdata;
		__buffer_offset += next_block_size;
	}

	memccpy_buffer_to_type(__buffer_offset, SPIRV_data_size);

	SPIRV_data.resize(static_cast<unsigned int>(SPIRV_data_size/4));
	std::memcpy(SPIRV_data.data(), &buffer[__buffer_offset], SPIRV_data_size);
	__buffer_offset += SPIRV_data_size;

	return 0;
}

unsigned int _ShaderFile_file_data::get_object_size() const
{
	unsigned int __buffer_size = 0;
	__buffer_size += sizeof(magic_number);
	__buffer_size += sizeof(shaders_count);
	__buffer_size += sizeof(shaders_byte_size);
	for (size_t i = 0; i < shaders.size(); i++)
	{
		__buffer_size += shaders[i].get_object_size();
	}

	return __buffer_size;
}

void _ShaderFile_file_data::write_to_buffer(std::vector<char>& buffer) const
{
	buffer.resize(get_object_size());
	unsigned int buffer_offset = 0;
	memccpy_type_to_buffer(buffer_offset, magic_number);
	memccpy_type_to_buffer(buffer_offset, shaders_count);
	memccpy_type_to_buffer(buffer_offset, shaders_byte_size);

	for (size_t i = 0; i < shaders_count; i++)
	{
		std::vector<char> __subbuffer_data;
		shaders[i].write_to_buffer(__subbuffer_data);
		std::memcpy(&buffer[buffer_offset], __subbuffer_data.data(), __subbuffer_data.size());
		buffer_offset += static_cast<unsigned int>(__subbuffer_data.size());
	}

}

int _ShaderFile_file_data::get_object_from_buffer(const std::vector<char>& buffer)
{
	if (buffer.size() < (sizeof(SHADER_FILE_HEADER) + sizeof(shaders_count) + sizeof(shaders_byte_size))) return 1;
	
	unsigned int __buffer_offset = 0;
	if(std::strcmp(&buffer[0], SHADER_FILE_HEADER) != 0) return -1;
	__buffer_offset += sizeof(SHADER_FILE_HEADER);

	memccpy_buffer_to_type(__buffer_offset, shaders_count);
	memccpy_buffer_to_type(__buffer_offset, shaders_byte_size);
	if (buffer.size() != (sizeof(SHADER_FILE_HEADER) + sizeof(shaders_count) + sizeof(shaders_byte_size) + shaders_byte_size)) return 1;
	shaders.resize(shaders_count);

	for (size_t i = 0; i < shaders_count; i++)
	{
		unsigned int next_block_size = 0;
		std::memcpy(&next_block_size, &buffer[__buffer_offset], sizeof(next_block_size));

		std::vector<char> _subbuffer(next_block_size);
		std::memcpy(&_subbuffer[0], &buffer[__buffer_offset], next_block_size);

		_ShaderModule_file_data _subdata;
		if(_subdata.get_object_from_buffer(_subbuffer) != 0) return 1;
		shaders[i] = _subdata;
		__buffer_offset += next_block_size;
	}

	return 0;
}

int read_shader_file(const std::string& filepath, _ShaderFile_file_data& data)
{

	size_t file_size = std::filesystem::file_size(filepath);
	std::vector<char> binary_data(file_size);
	auto file = std::ifstream(filepath, std::ios_base::binary);
	file.read(binary_data.data(), file_size);
	file.close();

	if (data.get_object_from_buffer(binary_data) != 0)
	{
		std::cout << "Failed to parse file: " << filepath << std::endl;
		return 1;
	}
	return 0;
}
