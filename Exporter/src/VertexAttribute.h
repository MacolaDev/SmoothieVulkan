#pragma once
#include <string>
#include <vector>
#include <Python.h>
#include <fstream>
#include "Math/SmoothieMath.h"

struct VertexAttributeNamesPerType 
{
	std::vector<const char*> floatNames;
	std::vector<const char*> intNames;
	std::vector<const char*> boolNames;
	std::vector<const char*> int8Names;

	std::vector<const char*> vec2FloatNames;
	std::vector<const char*> vec2IntNames;
	std::vector<const char*> vec3FloatNames;
	std::vector<const char*> vec4FloatNames;
	std::vector<const char*> vec4ByteNames;

};


struct VertexAttributeBase
{
	std::string attributeName;
	VertexAttributeBase(const std::string& name) : attributeName(name) {};
	virtual void writeToBuffer(size_t index, std::vector<char>& buffer) const {};
};


struct VertexAttributeFloat : public VertexAttributeBase
{
	std::vector<float> data;
	VertexAttributeFloat(const std::string& name, PyObject* pythonData, int& error);
	void writeToBuffer(size_t index, std::vector<char>& buffer) const override;
};

struct VertexAttributeVector2 : public VertexAttributeBase
{
	std::vector<SmoothieMath::Vector2> data;
	VertexAttributeVector2(const std::string& name, PyObject* pythonData, int& error);
	void writeToBuffer(size_t index, std::vector<char>& buffer) const override;
};

struct VertexAttributeVector3 : public VertexAttributeBase
{
	std::vector<SmoothieMath::Vector3> data;
	VertexAttributeVector3(const std::string& name, PyObject* pythonData, int& error);
	void writeToBuffer(size_t index, std::vector<char>& buffer) const override;
};

struct VertexAttributeVector4 : public VertexAttributeBase
{
	std::vector<SmoothieMath::Vector4> data;
	VertexAttributeVector4(const std::string& name, PyObject* pythonData, int& error);
	void writeToBuffer(size_t index, std::vector<char>& buffer) const override;
};

struct VertexAttributeInt : public VertexAttributeBase
{
	std::vector<int> data;
	VertexAttributeInt(const std::string& name, PyObject* pythonData, int& error);
	void writeToBuffer(size_t index, std::vector<char>& buffer) const override;
};

struct VertexAttributeInt8 : public VertexAttributeBase
{
	std::vector<char> data;
	VertexAttributeInt8(const std::string& name, PyObject* pythonData, int& error);
	void writeToBuffer(size_t index, std::vector<char>& buffer) const override;
};


int getVertexData(PyObject* listWithVertexData, VertexAttributeNamesPerType& namesPerType, std::vector<VertexAttributeBase*>& vertexAttributes);
void freeVertexAttributeData(std::vector<VertexAttributeBase*>& data);

int analyzeShaderFile(const std::string& filepath);