#include "exporter.h"
#include <iostream>
#include <string>

int packNormal(const vec3& normal) 
{
	int x_byte = int((1 + normal.x) * 127) & 0xFF;
	int y_byte = int((1 + normal.y) * 127) & 0xFF;
	int z_byte = int((1 + normal.z) * 127) & 0xFF;
	int out = (((x_byte << 0) | (y_byte << 8)) | (z_byte << 16)) | 0;
	return out;
}

int packNormal(const vec3& normal, int materialIndex)
{
	int x_byte = int((1 + normal.x) * 127) & 0xFF;
	int y_byte = int((1 + normal.y) * 127) & 0xFF;
	int z_byte = int((1 + normal.z) * 127) & 0xFF;
	int out = (((x_byte << 0) | (y_byte << 8)) | (z_byte << 16)) | (materialIndex << 24) | 0;
	return out;
}

int packColor(const vec4& color)
{
	int x_byte = int(color.x * 255) & 0xFF;
	int y_byte = int(color.y * 255) & 0xFF;
	int z_byte = int(color.z * 255) & 0xFF;
	int w_byte = int(color.w * 255) & 0xFF;

	int out = (((x_byte << 0) | (y_byte << 8)) | (z_byte << 16)) | (w_byte << 24) | 0;
	return out;
}


std::vector<vec3> PythonListTovec3(PyObject* pyList) 
{
	std::vector<vec3> output;
	for (int i = 0; i < PyList_Size(pyList) / 3; i++)
	{
		float x = (float)PyFloat_AsDouble(PyList_GetItem(pyList, 3*i));
		float y = (float)PyFloat_AsDouble(PyList_GetItem(pyList, 3*i+1));
		float z = (float)PyFloat_AsDouble(PyList_GetItem(pyList, 3*i+2));
		output.push_back({ x, y, z });
	}
	return output;
}

std::vector<vec4> PythonListTovec4(PyObject* pyList)
{
	std::vector<vec4> output;
	for (int i = 0; i < PyList_Size(pyList) / 4; i++)
	{
		float x = (float)PyFloat_AsDouble(PyList_GetItem(pyList, 4 * i));
		float y = (float)PyFloat_AsDouble(PyList_GetItem(pyList, 4 * i + 1));
		float z = (float)PyFloat_AsDouble(PyList_GetItem(pyList, 4 * i + 2));
		float w = (float)PyFloat_AsDouble(PyList_GetItem(pyList, 4 * i + 3));
		output.push_back({ x, y, z, w });
	}
	return output;
}



std::vector<vec2> PythonListTovec2(PyObject* pyList)
{
	std::vector<vec2> output;
	for (int i = 0; i < PyList_Size(pyList) / 2; i++)
	{
		float x = (float)PyFloat_AsDouble(PyList_GetItem(pyList, 2 * i));
		float y = (float)PyFloat_AsDouble(PyList_GetItem(pyList, 2 * i + 1));
		output.push_back({ x, y });
	}
	return output;
}

std::vector<long> PythonListToUCHAR(PyObject* pyList)
{
	std::vector<long> output;
	for (int i = 0; i < PyList_Size(pyList); i++)
	{
		char x = (char)PyLong_AsLong(PyList_GetItem(pyList, i));
		output.push_back(x);
	}
	return output;
}


PyObject* write_geometry_file(PyObject* self, PyObject* args)
{	
	//Get data from blender
	PyObject* geometryFilePath = PyObject_GetAttrString(args, "geometryFilePath");

	PyObject* numberOfvertices = PyObject_GetAttrString(args, "numberOfvertices");
	PyObject* vertexCoordinates = PyObject_GetAttrString(args, "vertexCoordinates");
	PyObject* vertexUVs = PyObject_GetAttrString(args, "vertexUVs");
	PyObject* vertexNormals = PyObject_GetAttrString(args, "vertexNormals");
	PyObject* vertexTangents = PyObject_GetAttrString(args, "vertexTangents");
	PyObject* vertexBitangents = PyObject_GetAttrString(args, "vertexBitangents");

	PyObject* numberOfIndices = PyObject_GetAttrString(args, "numberOfIndices");
	PyObject* indices = PyObject_GetAttrString(args, "indices");
	
	PyObject* materialIndices = PyObject_GetAttrString(args, "materialIndices");

	PyObject* use_vertex_colors = PyObject_GetAttrString(args, "use_vertex_colors");
	PyObject* vertexColors = PyObject_GetAttrString(args, "vertexColors");
	
	
	//Convert it to C++ types
	std::string file = PyUnicode_AsUTF8(geometryFilePath);
	int nOfVertices = (int)PyLong_AsLong(numberOfvertices);

	std::vector<vec3> coordinates = PythonListTovec3(vertexCoordinates);
	std::vector<vec2> UVs = PythonListTovec2(vertexUVs);
	std::vector<vec3> normals = PythonListTovec3(vertexNormals);
	std::vector<vec3> tangents = PythonListTovec3(vertexTangents);
	std::vector<vec3> bitangents = PythonListTovec3(vertexBitangents);
	
	std::vector<long> mat_indices = PythonListToUCHAR(materialIndices);
	std::vector<vec4> colors;
	int nOfIndices = (int)PyLong_AsLong(numberOfIndices);
	bool use_colors = static_cast<bool>(PyObject_IsTrue(use_vertex_colors));
	
	if (use_colors) 
	{
		colors = PythonListTovec4(vertexColors);
		std::cout << "Use colors!" << std::endl;
	}

	//Create vertex buffer
	std::vector<XYZNUVTB> buffer;
	
	VertexBufferBase* vertexBuffer = nullptr;
	if (use_colors) 
	{
		vertexBuffer = new VertexBuffer<XYZNUVTBC>;
		for (int i = 0; i < nOfVertices; i++)
		{
			XYZNUVTBC vertex;
			vertex.coordinate.x = coordinates[i].x;
			vertex.coordinate.y = coordinates[i].y;
			vertex.coordinate.z = coordinates[i].z;

			vertex.normal = packNormal(normals[i]) | (mat_indices[i] << 24);

			vertex.uv = UVs[i];
			vertex.tangent = packNormal(tangents[i]);
			vertex.binormal = packNormal(bitangents[i]);
			vertex.color = packColor(colors[i]);

			vertexBuffer->insertVertex(&vertex);
		}
	
	}

	else 
	{
		vertexBuffer = new VertexBuffer<XYZNUVTB>;
		for (int i = 0; i < nOfVertices; i++)
		{
			XYZNUVTB vertex;
			vertex.coordinate.x = coordinates[i].x;
			vertex.coordinate.y = coordinates[i].y;
			vertex.coordinate.z = coordinates[i].z;

			vertex.normal = packNormal(normals[i]) | (mat_indices[i] << 24);
		
			vertex.uv = UVs[i];
			vertex.tangent = packNormal(tangents[i]);
			vertex.binormal = packNormal(bitangents[i]);
		
			buffer.push_back(vertex);
			vertexBuffer->insertVertex(&vertex);
		} 
	}


	//Create index buffer
	//TODO: make reading from py list in seperate funcion
	std::vector<unsigned int> indexes;
	for (int i = 0; i < nOfIndices; i++)
	{
		indexes.push_back(PyLong_AsLong(PyList_GetItem(indices, i)));
	}


	//Output file
	std::ofstream outFile(file, std::ios::binary);
	if (!outFile) 
	{
		std::cout << "Can't open file " << file << " for writing data!" << std::endl;
		return Py_None;
	}
	
	std::cout << "Writing file: " << file << std::endl;

	int vertexBufferOffset = 20;
	int vertexBufferType = 0; //0 for XYZNUVTB
	if (use_colors) { vertexBufferType = 1; }
	int vertexBufferSize = vertexBuffer->bufferSize();//(int)buffer.size() * sizeof(XYZNUVTB);
	int indexBufferSize = (int)indexes.size() * sizeof(unsigned int);
	int indexBufferOffset = vertexBufferOffset + vertexBufferSize;
	
	//Vertex buffer type
	//TODO: change it to other types aswell
	outFile.write(reinterpret_cast<char*>(&vertexBufferType), sizeof(int));

	//Nubmer of vertices
	outFile.write(reinterpret_cast<char*>(&nOfVertices), sizeof(int));

	//Vertex pointer
	outFile.write(reinterpret_cast<char*>(&vertexBufferOffset), sizeof(int));

	//Number of indices
	outFile.write(reinterpret_cast<char*>(&nOfIndices), sizeof(int));

	//Index pointer
	outFile.write(reinterpret_cast<char*>(&indexBufferOffset), sizeof(int));

	//Write vertex buffer
	//outFile.write(reinterpret_cast<char*>(buffer.data()), vertexBufferSize);
	vertexBuffer->writeToFile(outFile);

	//Write index buffer
	outFile.write(reinterpret_cast<char*>(indexes.data()), indexBufferSize);

	outFile.close();
	delete vertexBuffer;
	return Py_None;
}