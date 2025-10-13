#include "DescriptorType.h"

using namespace Smoothie;

static unsigned long long hash_name(const std::string& name) 
{
	const unsigned long long FNV_OFFSET_BASIS = 0xcbf29ce484222325;
	const unsigned long long FNV_PRIME = 0x100000001b3;

	unsigned long long hash = FNV_PRIME;
	for (unsigned long long i = 0; i < name.size(); i++)
	{
		hash ^= name[i];
		hash *= FNV_PRIME;
		hash &= 0xFFFFFFFFFFFFFFFF;
	}

	return hash;
}

BufferTypeBufferData Smoothie::BufferTypeBase::getBufferData()
{
	BufferTypeBufferData _data;
	_data.bufferOffset = typeBufferOffset;
	_data.data = data;
	_data.typeSize = typeSize;
	return _data;
}

Smoothie::BufferTypeBase::BufferTypeBase(int offset, const std::string& prop_name)
{
	name = prop_name;
	name_hash = hash_name(prop_name);
	typeBufferOffset = offset;
}

Smoothie::BufferType_bool::BufferType_bool(int offset, const std::string& prop_name, bool value) : BufferTypeBase(offset, prop_name)
{
	this->value = value;
	type = MODEL_DESCRIPTOR_DATA_TYPE_BOOL;
	typeSize = sizeof(int);
	data = (void*)&value;
}

Smoothie::BufferType_bvec2::BufferType_bvec2(int offset, const std::string& prop_name, bool value1, bool value2)
{
	this->value = { value1, value2 };
	type = MODEL_DESCRIPTOR_DATA_TYPE_BOOL_VEC2;
	typeSize = sizeof(bvec2_value);
	data = (void*)&value;
}

Smoothie::BufferType_bvec3::BufferType_bvec3(int offset, const std::string& prop_name, bool value1, bool value2, bool value3)
{
	this->value = { value1, value2, value3 };
	type = MODEL_DESCRIPTOR_DATA_TYPE_BOOL_VEC3;
	typeSize = sizeof(bvec3_value);
	data = (void*)&value;
}

Smoothie::BufferType_bvec4::BufferType_bvec4(int offset, const std::string& prop_name, bool value1, bool value2, bool value3, bool value4)
{
	this->value = { value1, value2, value3, value4 };
	type = MODEL_DESCRIPTOR_DATA_TYPE_BOOL_VEC4;
	typeSize = sizeof(bvec4_value);
	data = (void*)&value;
}

Smoothie::BufferType_float::BufferType_float(int offset, const std::string& prop_name, float value)
{
	this->value = value;
	type = MODEL_DESCRIPTOR_DATA_TYPE_FLOAT;
	typeSize = sizeof(float);
	data = (void*)&value;
}

Smoothie::BufferType_vec2::BufferType_vec2(int offset, const std::string& prop_name, float value1, float value2)
{
	this->value = { value1, value2 };
	type = MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_VEC2;
	typeSize = sizeof(vec2);
	data = (void*)&value;
}

Smoothie::BufferType_vec3::BufferType_vec3(int offset, const std::string& prop_name, float value1, float value2, float value3)
{
	this->value = { value1, value2, value3 };
	type = MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_VEC3;
	typeSize = sizeof(vec3);
	data = (void*)&value;
}

Smoothie::BufferType_vec4::BufferType_vec4(int offset, const std::string& prop_name, float value1, float value2, float value3, float value4)
{
	this->value = { value1, value2, value3, value4 };
	type = MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_VEC4;
	typeSize = sizeof(vec4);
	data = (void*)&value;
}

Smoothie::BufferType_double::BufferType_double(int offset, const std::string& prop_name, double value)
{
	this->value = value;
	type = MODEL_DESCRIPTOR_DATA_TYPE_DOUBLE;
	typeSize = sizeof(double);
	data = (void*)&value;
}

Smoothie::BufferType_dvec2::BufferType_dvec2(int offset, const std::string& prop_name, double value1, double value2)
{
	this->value = { value1, value2 };
	type = MODEL_DESCRIPTOR_DATA_TYPE_DOUBLE_VEC2;
	typeSize = sizeof(dvec2);
	data = (void*)&value;
}

Smoothie::BufferType_dvec3::BufferType_dvec3(int offset, const std::string& prop_name, double value1, double value2, double value3)
{
	this->value = { value1, value2, value3 };
	type = MODEL_DESCRIPTOR_DATA_TYPE_DOUBLE_VEC3;
	typeSize = sizeof(dvec3);
	data = (void*)&value;
}

Smoothie::BufferType_dvec4::BufferType_dvec4(int offset, const std::string& prop_name, double value1, double value2, double value3, double value4)
{
	this->value = { value1, value2, value3, value4 };
	type = MODEL_DESCRIPTOR_DATA_TYPE_DOUBLE_VEC4;
	typeSize = sizeof(dvec4);
	data = (void*)&value;
}

Smoothie::BufferType_int::BufferType_int(int offset, const std::string& prop_name, int value)
{
	this->value = value;
	type = MODEL_DESCRIPTOR_DATA_TYPE_INT;
	typeSize = sizeof(int);
	data = (void*)&value;
}

Smoothie::BufferType_ivec2::BufferType_ivec2(int offset, const std::string& prop_name, int value1, int value2)
{
	this->value = { value1, value2 };
	type = MODEL_DESCRIPTOR_DATA_TYPE_INT_VEC2;
	typeSize = sizeof(ivec2);
	data = (void*)&value;
}

Smoothie::BufferType_ivec3::BufferType_ivec3(int offset, const std::string& prop_name, int value1, int value2, int value3)
{
	this->value = { value1, value2, value3 };
	type = MODEL_DESCRIPTOR_DATA_TYPE_INT_VEC3;
	typeSize = sizeof(ivec3);
	data = (void*)&value;
}

Smoothie::BufferType_ivec4::BufferType_ivec4(int offset, const std::string& prop_name, int value1, int value2, int value3, int value4)
{
	this->value = { value1, value2, value3, value4 };
	type = MODEL_DESCRIPTOR_DATA_TYPE_INT_VEC4;
	typeSize = sizeof(ivec4);
	data = (void*)&value;
}

Smoothie::BufferType_uint::BufferType_uint(int offset, const std::string& prop_name, unsigned int value)
{
	this->value = value;
	type = MODEL_DESCRIPTOR_DATA_TYPE_UNSIGNED_INT;
	typeSize = sizeof(unsigned int);
	data = (void*)&value;
}

Smoothie::BufferType_uvec2::BufferType_uvec2(int offset, const std::string& prop_name, unsigned int value1, unsigned int value2)
{
	this->value = { value1, value2};
	type = MODEL_DESCRIPTOR_DATA_TYPE_UNSIGNED_INT_VEC2;
	typeSize = sizeof(uvec2);
	data = (void*)&value;
}

Smoothie::BufferType_uvec3::BufferType_uvec3(int offset, const std::string& prop_name, unsigned int value1, unsigned int value2, unsigned int value3)
{
	this->value = { value1, value2, value3 };
	type = MODEL_DESCRIPTOR_DATA_TYPE_UNSIGNED_INT_VEC3;
	typeSize = sizeof(uvec3);
	data = (void*)&value;
}

Smoothie::BufferType_uvec4::BufferType_uvec4(int offset, const std::string& prop_name, unsigned int value1, unsigned int value2, unsigned int value3, unsigned int value4)
{
	this->value = { value1, value2, value3, value4 };
	type = MODEL_DESCRIPTOR_DATA_TYPE_UNSIGNED_INT_VEC4;
	typeSize = sizeof(uvec4);
	data = (void*)&value;
}

Smoothie::BufferType_mat2::BufferType_mat2(int offset, const std::string& prop_name, const SmoothieMath::Vector2& column0, SmoothieMath::Vector2 column1)
{
	this->value.column0 = column0;
	this->value.column1 = column1;
	type = MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_MAT2;
	typeSize = sizeof(mat2);
	data = (void*)&value;
}

Smoothie::BufferType_mat3::BufferType_mat3(int offset, const std::string& prop_name, const SmoothieMath::Matrix3x3& mat)
{
	this->value.column0 = mat.column0;
	this->value.column1 = mat.column1;
	this->value.column2 = mat.column2;
	type = MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_MAT3;
	typeSize = sizeof(mat3);
	data = (void*)&value;
}

Smoothie::BufferType_mat4::BufferType_mat4(int offset, const std::string& prop_name, const SmoothieMath::Matrix4x4& mat)
{
	this->value = mat;
	type = MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_MAT4;
	typeSize = sizeof(SmoothieMath::Matrix4x4);
	data = (void*)&value;
}
