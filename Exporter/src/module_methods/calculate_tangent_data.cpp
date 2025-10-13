#include "exporter.h"
#include <vector>
#include <iostream>
#include <algorithm>

struct TANGENT_DATA {
	int index;
	vec3 vector;
	
	bool operator == (TANGENT_DATA& other) 
	{
		if (other.index == index) 
		{
			return true;
		}
		else 
		{
			return false;
		}
	}
};

vec3 averageVectors(const std::vector<vec3>& toAverageList) 
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f; //Preventing division with 0
	
	for (int i = 0; i < toAverageList.size(); i++) 
	{
		x += toAverageList[i].x;
		y += toAverageList[i].y;
		z += toAverageList[i].z;
	}
	float length = sqrt(x * x + y * y + z * z);
	if (length == 0) 
	{
		length = 1.0f;
	}
	x /= length;
	y /= length;
	z /= length;
	return { x, y, z };
}


PyObject* calculate_tangent_data(PyObject* self, PyObject* args)
{
	//Indexes from blender
	PyObject* indexes_list = PyObject_GetAttrString(args, "indexes");
	if (indexes_list == nullptr)
	{
		std::cout << "SmoothieExporter: calculate_tangent_data: input object must have list attribute \"indexes\"!" << std::endl;
		return Py_None;
	}


	//Data from blender
	PyObject* data_list = PyObject_GetAttrString(args, "data");
	if (data_list == nullptr)
	{
		std::cout << "SmoothieExporter: calculate_tangent_data: input object must have list attribute \"data\"!" << std::endl;
		return Py_None;
	}

	//TODO: Made types check for each element
	
	//Checking if lists match data
	if ((3 * PyList_Size(indexes_list)) != PyList_Size(data_list))
	{
		std::cout << __FUNCTION__": List's Length dont match!" << std::endl;
		std::cout << PyList_Size(indexes_list) << " " << PyList_Size(data_list) << std::endl;
		return Py_None;
	}

	std::vector<TANGENT_DATA> dataFromBlender;
	for (int i = 0; i < (int)PyList_Size(indexes_list); i++)
	{
		int index = (int)PyLong_AsLong(PyList_GetItem(indexes_list, i));

		float x = (float)PyFloat_AsDouble(PyList_GetItem(data_list, 3 * i));
		float y = (float)PyFloat_AsDouble(PyList_GetItem(data_list, 3 * i + 1));
		float z = (float)PyFloat_AsDouble(PyList_GetItem(data_list, 3 * i + 2));
		//std::cout << i << " " << index << " " << x << " " << y << " " << z << std::endl;
		dataFromBlender.push_back({ index, {x, y , z} });
	}

	std::sort(dataFromBlender.begin(), dataFromBlender.end(), [](const TANGENT_DATA& a, const TANGENT_DATA& b) {
		return a.index < b.index;
		});
	
	std::vector<vec3> toAverage;
	std::vector<vec3> averaged;
	for (int i = 0; i < dataFromBlender.size(); i++) 
	{
		TANGENT_DATA data = dataFromBlender[i];
		TANGENT_DATA next = dataFromBlender[i + 1];
		
		/*std::cout << i << " " << dataFromBlender[i].index << " "
			<< dataFromBlender[i].vector.x << " "
			<< dataFromBlender[i].vector.y << " "
			<< dataFromBlender[i].vector.z << " "
			<< std::endl;*/
		
		if (data == next) 
		{
			toAverage.push_back(data.vector);
		}
		else 
		{
			toAverage.push_back(data.vector);
			averaged.push_back(averageVectors(toAverage));
			toAverage.clear();
		}
	}
	
	PyObject* output = PyList_New(averaged.size() * 3);
	for (int i = 0; i < averaged.size(); i++) 
	{
		PyObject* x = PyFloat_FromDouble(averaged[i].x);
		PyObject* y = PyFloat_FromDouble(averaged[i].y);
		PyObject* z = PyFloat_FromDouble(averaged[i].z);

		PyList_SetItem(output, 3*i, x);
		PyList_SetItem(output, 3*i + 1, y);
		PyList_SetItem(output, 3*i + 2, z);

	}

	return output;
}