#pragma once
#include <string>
#include <Core/Constants.h>
#include <Math/SmoothieMath.h>

namespace Smoothie 
{
	//This is data used for filling up a buffer object
	struct BufferTypeBufferData
	{
		int bufferOffset = -1;
		int typeSize = -1;
		void* data = nullptr;
	};

	//Base type of a object inside a buffer.
	struct BufferTypeBase
	{
		ModelDescriptorDataType type = MODEL_DESCRIPTOR_DATA_TYPE_NO_TYPE;
		int typeBufferOffset = -1;
		int typeSize = -1;
		unsigned long long name_hash = 0;
		std::string name;
		BufferTypeBufferData getBufferData();
		void* data = nullptr;

		BufferTypeBase() = default;
		BufferTypeBase(int offset, const std::string& prop_name);
	};

	//bool type in gl, base alligment 4 bytes
	struct BufferType_bool : public BufferTypeBase 
	{
		int value = 0;
		BufferType_bool(int offset, const std::string& prop_name, bool value);
	};

	//bvec2 type in gl, base alligment 8 bytes
	struct BufferType_bvec2 : public BufferTypeBase
	{
		struct bvec2_value { int value1 = 0, value2 = 0; };
		bvec2_value value;
		BufferType_bvec2(int offset, const std::string& prop_name, bool value1, bool value2);
	};

	//bvec3 type in gl, base alligment 16 bytes
	struct BufferType_bvec3 : public BufferTypeBase
	{
		struct bvec3_value { int value1 = 0, value2 = 0, value3 = 0, padding = 0; };
		bvec3_value value;
		BufferType_bvec3(int offset, const std::string& prop_name, bool value1, bool value2, bool value3);
	};

	//bvec4 type in gl, base alligment 16 bytes
	struct BufferType_bvec4 : public BufferTypeBase
	{
		struct bvec4_value { int value1 = 0, value2 = 0, value3 = 0, value4 = 0; };
		bvec4_value value;
		BufferType_bvec4(int offset, const std::string& prop_name, bool value1, bool value2, bool value3, bool value4);
	};

	//float type in gl, base alligment 4 bytes
	struct BufferType_float : public BufferTypeBase
	{
		float value;
		BufferType_float(int offset, const std::string& prop_name, float value);
	};

	//vec2 type in gl, base alligment 8 bytes
	struct BufferType_vec2 : public BufferTypeBase
	{
		struct vec2 { float value1 = 0, value2 = 0; };
		vec2 value;
		BufferType_vec2(int offset, const std::string& prop_name, float value1, float value2);
	};

	//vec3 type in gl, base alligment 16 bytes
	struct BufferType_vec3 : public BufferTypeBase
	{
		struct vec3 { float value1 = 0, value2 = 0, value3 = 0, padding = 0; };
		vec3 value;
		BufferType_vec3(int offset, const std::string& prop_name, float value1, float value2, float value3);
	};

	//vec4 type in gl, base alligment 16 bytes
	struct BufferType_vec4 : public BufferTypeBase
	{
		struct vec4 { float value1 = 0, value2 = 0, value3 = 0, value4 = 0; };
		vec4 value;
		BufferType_vec4(int offset, const std::string& prop_name, float value1, float value2, float value3, float value4);
	};

	//double type in gl, base alligment 8 bytes
	struct BufferType_double : public BufferTypeBase
	{
		double value;
		BufferType_double(int offset, const std::string& prop_name, double value);
	};

	//dvec2 type in gl, base alligment 16 bytes
	struct BufferType_dvec2 : public BufferTypeBase
	{
		struct dvec2 { double value1 = 0, value2 = 0; };
		dvec2 value;
		BufferType_dvec2(int offset, const std::string& prop_name, double value1, double value2);
	};
	
	//dvec3 type in gl, base alligment 16 bytes
	struct BufferType_dvec3 : public BufferTypeBase
	{
		struct dvec3 { double value1 = 0, value2 = 0, value3 = 0, padding = 0; };
		dvec3 value;
		BufferType_dvec3(int offset, const std::string& prop_name, double value1, double value2, double value3);
	};

	//dvec3 type in gl, base alligment 16 bytes
	struct BufferType_dvec4 : public BufferTypeBase
	{
		struct dvec4 { double value1 = 0, value2 = 0, value3 = 0, value4 = 0; };
		dvec4 value;
		BufferType_dvec4(int offset, const std::string& prop_name, double value1, double value2, double value3, double value4);
	};

	//int type in gl, base alligment 4 bytes
	struct BufferType_int : public BufferTypeBase
	{
		int value = 0;
		BufferType_int(int offset, const std::string& prop_name, int value);
	};

	//ivec2 type in gl, base alligment 8 bytes
	struct BufferType_ivec2 : public BufferTypeBase
	{
		struct ivec2 { int value1 = 0, value2 = 0; };
		ivec2 value;
		BufferType_ivec2(int offset, const std::string& prop_name, int value1, int value2);
	};

	//ivec3 type in gl, base alligment 16 bytes
	struct BufferType_ivec3 : public BufferTypeBase
	{
		struct ivec3 { int value1 = 0, value2 = 0, value3 = 0, padding = 0; };
		ivec3 value;
		BufferType_ivec3(int offset, const std::string& prop_name, int value1, int value2, int value3);
	};

	//ivec4 type in gl, base alligment 16 bytes
	struct BufferType_ivec4 : public BufferTypeBase
	{
		struct ivec4 { int value1 = 0, value2 = 0, value3 = 0, value4 = 0; };
		ivec4 value;
		BufferType_ivec4(int offset, const std::string& prop_name, int value1, int value2, int value3, int value4);
	};

	//unt type in gl, base alligment 4 bytes
	struct BufferType_uint : public BufferTypeBase
	{
		unsigned int value = 0;
		BufferType_uint(int offset, const std::string& prop_name, unsigned int value);
	};

	//uvec2 type in gl, base alligment 8 bytes
	struct BufferType_uvec2 : public BufferTypeBase
	{
		struct uvec2 { unsigned int value1 = 0, value2 = 0; };
		uvec2 value;
		BufferType_uvec2(int offset, const std::string& prop_name, unsigned int value1, unsigned int value2);
	};

	//uvec3 type in gl, base alligment 16 bytes
	struct BufferType_uvec3 : public BufferTypeBase
	{
		struct uvec3 { unsigned int value1 = 0, value2 = 0, value3 = 0, padding = 0; };
		uvec3 value;
		BufferType_uvec3(int offset, const std::string& prop_name, unsigned int value1, unsigned int value2, unsigned int value3);
	};

	//uvec4 type in gl, base alligment 16 bytes
	struct BufferType_uvec4 : public BufferTypeBase
	{
		struct uvec4 { unsigned int value1 = 0, value2 = 0, value3 = 0, value4 = 0; };
		uvec4 value;
		BufferType_uvec4(int offset, const std::string& prop_name, unsigned int value1, unsigned int value2, unsigned int value3, unsigned int value4);
	};

	//mat2 type in gl, base 16 bytes per column
	struct BufferType_mat2 : public BufferTypeBase
	{
		struct mat2 { 
			SmoothieMath::Vector2 column0, padding0, column1, padding1;
		};
		mat2 value;
		BufferType_mat2(int offset, const std::string& prop_name, const SmoothieMath::Vector2& column0, SmoothieMath::Vector2 column1);
	};

	//mat3 type in gl, base alligment 16 bytes per column
	struct BufferType_mat3 : public BufferTypeBase
	{
		struct mat3 
		{
			SmoothieMath::Vector3 column0; float padding0 = 0; 
			SmoothieMath::Vector3 column1; float padding1 = 0;
			SmoothieMath::Vector3 column2; float padding2 = 0;
		};
		mat3 value;
		BufferType_mat3(int offset, const std::string& prop_name, const SmoothieMath::Matrix3x3& mat);
	};

	//mat4 type in gl, base alligment 16 bytes per column
	struct BufferType_mat4 : public BufferTypeBase
	{
		SmoothieMath::Matrix4x4 value;
		BufferType_mat4(int offset, const std::string& prop_name, const SmoothieMath::Matrix4x4& mat);
	};
}