#pragma once
#include <string>
#include <vector>
#include "Core/Constants.h"

#define SHADER_FILE_HEADER "301"

struct _File_Writtable
{
	unsigned int _block_size = 0;
	virtual unsigned int get_object_size() const = 0;
	virtual void write_to_buffer(std::vector<char>& buffer) const = 0;
	virtual int get_object_from_buffer(const std::vector<char>& buffer) = 0;

	_File_Writtable() = default;

	void calculate_block_size() { _block_size = get_object_size(); }
};

enum class _Reflection_type_file_data
{
	NO_TYPE = 0,
	PIPE_INPUT = 1,
	PIPE_OUTPUT = 2,
	UNIFORM_BLOCK = 3,
	UNIFORM_VARIABLE = 4,
	STORAGE_BLOCK = 5,
	STORAGE_VARIABLE = 6
};

//File data for creating descriptor set data for shader module
struct _ShaderDescriptorsReflection_file_data: public _File_Writtable
{
	int offset;
	ModelDescriptorDataType data_type;
	_Reflection_type_file_data reflection_type;
	int size;                   // data size in bytes for a block, array size for a (non-block) object that's an array
	int index;
	int counterIndex;
	int numMembers;
	int arrayStride;            // stride of an array variable
	int topLevelArraySize;      // size of the top-level variable in a storage buffer member
	int topLevelArrayStride;    // stride of the top-level variable in a storage buffer member
	unsigned int stages;
	
	int location;
	int binding;
	int set;
	int owning_set; //If its a buffer variable, this is the owning set that contains that buffer
	int owning_set_binding; //If its a buffer variable, this is the owning set binding contains that buffer

	int name_size;
	std::string name;

	unsigned int get_object_size() const override;
	void write_to_buffer(std::vector<char>& buffer) const override;
	int get_object_from_buffer(const std::vector<char>& buffer) override;
};

//File data for creating ShaderModule
struct _ShaderModule_file_data: public _File_Writtable
{
	unsigned int shader_name_size = 0;
	std::string shader_name;

	unsigned int shader_stage = -1;
	 
	unsigned int reflections_count = 0;
	unsigned int reflections_byte_size = 0;
	std::vector<_ShaderDescriptorsReflection_file_data> reflections;

	unsigned int SPIRV_data_size = 0;
	std::vector<unsigned int> SPIRV_data;
	
	unsigned int get_object_size() const override;
	void write_to_buffer(std::vector<char>& buffer) const override;
	int get_object_from_buffer(const std::vector<char>& buffer) override;

	_ShaderModule_file_data() = default;
};

struct _ShaderFile_file_data: public _File_Writtable
{
	const char magic_number[4] = SHADER_FILE_HEADER;
	unsigned int shaders_count = 0;
	unsigned int shaders_byte_size = 0;
	std::vector<_ShaderModule_file_data> shaders;

	virtual unsigned int get_object_size() const;
	virtual void write_to_buffer(std::vector<char>& buffer) const;
	int get_object_from_buffer(const std::vector<char>& buffer) override;
};

int read_shader_file(const std::string& filepath, _ShaderFile_file_data& data);