#pragma once
#include "Python.h"
#include <vector>
#include <fstream>
#include <iostream>
struct vec4
{
	float x, y, z, w;
};

struct vec3
{
	float x, y, z;
};

struct vec2
{
	float x, y;
};

//TODO: make this better
enum VertexBufferType
{
	xyznuvtb = 0,
	xyznuvtbc =	1
};

struct VertexBufferBase 
{
public:
	virtual char* dataPointer() { return nullptr; };
	virtual size_t bufferSize() const { return 0;};
	virtual void writeToFile(std::ofstream& file) {};
	virtual void insertVertex(void* vertex) { std::cout << "Called inside: " << __FUNCTION__ << std::endl; };
};


template<class T> class VertexBuffer: public VertexBufferBase
{
	std::vector<T> bufferData;
	
public:

	VertexBuffer() = default;

	char* dataPointer() override
	{
		return reinterpret_cast<char*>(bufferData.data());
	};

	void insertVertex(void* vertex) override
	{
		bufferData.push_back(*(reinterpret_cast<T*>(vertex)));
	}

	size_t bufferSize() const override
	{
		return bufferData.size() * sizeof(T);
	};

	void writeToFile(std::ofstream& file) override 
	{
		file.write(dataPointer(), bufferSize());
	};
};

struct XYZNUVTB
{
	vec3 coordinate;
	int normal;
	vec2 uv;
	int tangent, binormal;
};

struct XYZNUVTBC
{
	vec3 coordinate;
	int normal;
	vec2 uv;
	int tangent, binormal;
	int color;
};



PyObject* calculate_uv_data(PyObject* self, PyObject* args);
const char calculate_uv_data_docs[] =
"Calculates UV data per vertex from per loop data.\n"
"Input object can be any object, as long as it has two list attribute named *indexes* and *data* with the same number of elements.\n"
"Returns list with per-vertex values.";

PyObject* calculate_tangent_data(PyObject* self, PyObject* args);
const char calculate_tangent_data_docs[] =
"Calculates normal/tangent/bitangent data per vertex from per loop data.\n"
"Input object can be any object, as long as it has two list attribute named *indexes* and *data* with the same number of elements.\n"
"Returns list with per-vertex values.";

PyObject* write_geometry_file(PyObject* self, PyObject* args);
const char write_geometry_file_data_docs[] =
"Takes vertex and index data and writes .geometry file";

PyObject* calculate_uv_data_averaged(PyObject* self, PyObject* args);
const char calculate_uv_data_averaged_docs[] =
"Similar to \"calculate_uv_data\" but it averages UV data from all loops for each vertex.";

PyObject* calculate_mat_indexes(PyObject* self, PyObject* args);
const char calculate_mat_indexes_docs[] =
"Expects to recieve object with list attributes \"vertex_index\" and \"material_index\".\n"
"Returns list with material index for each vertex.\n"
"WARNING: Function assumes that one material_index index has 3 vertices that make up that triangle aka len(material_index) == 3 * len(vertex_index)."
"Check \"calculate_mat_indexes_test.py\" to see proper use of this function.";

