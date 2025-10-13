#include "exporter.h"
#include <iostream>
#include <vector>
#include <algorithm>

struct UV_DATA 
{
	int index;
	vec2 UV;
};

vec2 averageVectors(const std::vector<vec2>& toAverageList)
{
	float x = 0.0f;
	float y = 0.0f;

	for (int i = 0; i < toAverageList.size(); i++)
	{
		x += toAverageList[i].x;
		y += toAverageList[i].y;
	}
	float length = sqrt(x * x + y * y);
	if (length == 0)
	{
		length = 1.0f;
	}
	x /= length;
	y /= length;
	return {x, y};
}


PyObject* calculate_uv_data_averaged(PyObject* self, PyObject* args)
{
	//Indexes from blender
	PyObject* indexes_list = PyObject_GetAttrString(args, "indexes");
	if (indexes_list == nullptr)
	{
		std::cout << "SmoothieExporter: calculate_uv_data_averaged: input object must have list attribute \"indexes\"!" << std::endl;
		return Py_None;
	}

	//Data from blender
	PyObject* data_list = PyObject_GetAttrString(args, "data");
	if (data_list == nullptr)
	{
		std::cout << "SmoothieExporter: calculate_uv_data_averaged: input object must have list attribute \"data\"!" << std::endl;
		return Py_None;
	}

	//TODO: Made types check for each element

	//Checking if lists match data
	if ((2 * PyList_Size(indexes_list)) != PyList_Size(data_list))
	{
		std::cout << __FUNCTION__": List's Length dont match!" << std::endl;
		std::cout << PyList_Size(indexes_list) << " " << PyList_Size(data_list) << std::endl;
		return Py_None;
	}

	int numberOfLoops = (int)PyList_Size(indexes_list);

	std::vector<UV_DATA> dataFromBlender;
	
	for (int i = 0; i < numberOfLoops; i++)
	{
		int index = (int)PyLong_AsLong(PyList_GetItem(indexes_list, i));

		float x = (float)PyFloat_AsDouble(PyList_GetItem(data_list, 2 * i));
		float y = (float)PyFloat_AsDouble(PyList_GetItem(data_list, 2 * i + 1));
		dataFromBlender.push_back({ index, {x, y} });
		
	}


	//Sort dataFomBlender with index, from lowest to highest
	std::sort(dataFromBlender.begin(), dataFromBlender.end(), [](const UV_DATA& a, const UV_DATA& b) {
		return a.index < b.index;
		});

	std::vector<vec2> toAverage;
	std::vector<vec2> averaged;
	for (int i = 0; i < dataFromBlender.size(); i++)
	{
		UV_DATA data = dataFromBlender[i];
		UV_DATA next = dataFromBlender[i + 1];
		//std::cout << data.index << "  " << data.UV.x << " " << data.UV.y << std::endl;
		if (data.index == next.index)
		{
			continue;
			//toAverage.push_back(data.UV);
		}
		else
		{
			averaged.push_back(data.UV);
			//averaged.push_back(averageVectors(toAverage));
			//toAverage.clear();
		}
	}

	PyObject* output = PyList_New(averaged.size() * 2);
	for (int i = 0; i < averaged.size(); i++)
	{
		PyObject* x = PyFloat_FromDouble(averaged[i].x);
		PyObject* y = PyFloat_FromDouble(averaged[i].y);
		PyList_SetItem(output, 2 * i, x);
		PyList_SetItem(output, 2 * i + 1, y);
	}

	return output;
}