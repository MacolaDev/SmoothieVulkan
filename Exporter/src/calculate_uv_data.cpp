#include "exporter.h"
#include <iostream>
#include <vector>
#include <algorithm> 

struct UV_DATA{
	int index;
	float x, y;
	
	bool operator == (const UV_DATA& other) 
	{
		if ((other.index == index) and (other.x == x) and (other.y == y))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};


PyObject* calculate_uv_data(PyObject* self, PyObject* args) 
{
	//Indexes from blender
	PyObject* indexes_list = PyObject_GetAttrString(args, "indexes");
	if (indexes_list == nullptr)
	{
		std::cout << "SmoothieExporter: calculate_uv_data: input object must have list attribute \"indexes\"!" << std::endl;
		return Py_None;
	}

	
	//Data from blender
	PyObject* data_list = PyObject_GetAttrString(args, "data");
	if (data_list == nullptr)
	{
		std::cout << "SmoothieExporter: calculate_uv_data: input object must have list attribute \"data\"!" << std::endl;
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

		float x = (float)PyFloat_AsDouble(PyList_GetItem(data_list, 2*i));
		float y = (float)PyFloat_AsDouble(PyList_GetItem(data_list, 2*i+1));
		dataFromBlender.push_back({ index, x, y });
	}
	
	//Sort dataFomBlender with index, from lowest to highest
	std::sort(dataFromBlender.begin(), dataFromBlender.end(), [](const UV_DATA& a, const UV_DATA& b) {
		return a.index < b.index;
		});
	
	//TODO: Fix this shit
	std::vector<UV_DATA> finalList;
	for (int i = 0; i < dataFromBlender.size(); i++) {
		if (dataFromBlender[i] == dataFromBlender[i + 1])
		{
			continue;
		}
		else {
			finalList.push_back(dataFromBlender[i]);
		}
	}

	PyObject* output = PyList_New(finalList.size() * 2);
	for (int i = 0; i < finalList.size(); i++) 
	{
		PyObject* x = PyFloat_FromDouble(finalList[i].x);
		PyObject* y = PyFloat_FromDouble(finalList[i].y);
		PyList_SetItem(output, 2*i, x);
		PyList_SetItem(output, 2*i+1, y);
	}
	
	return output;
}