#include "exporter.h"
#include <iostream>
#include <vector>
#include <algorithm>

struct mat_index 
{
	long vertex_index;
	long mat_index;
};


PyObject* calculate_mat_indexes(PyObject* self, PyObject* args)
{
	PyObject* vertex_indexes = PyObject_GetAttrString(args, "vertex_index");
	if (vertex_indexes == nullptr)
	{
		std::cout << "SmoothieExporter: calculate_mat_indexes: input object must have list attribute \"vertex_index\"!" << std::endl;
		return Py_None;
	}

	PyObject* material_indexes = PyObject_GetAttrString(args, "material_index");
	if (material_indexes == nullptr)
	{
		std::cout << "SmoothieExporter: calculate_mat_indexes: input object must have list attribute \"material_index\"!SmoothieExporter: calculate_mat_indexes: input object must have list attribute \"material_index\"!" << std::endl;
		return Py_None;
	}

	if ((3 * PyList_Size(material_indexes)) != PyList_Size(vertex_indexes))
	{
		std::cout << "SmoothieExporter: calculate_mat_indexes: Input lists are not valid! List sizes do not match! " << std::endl;
		return Py_None;
	}

	std::vector<mat_index> toSort;
	for (int i = 0; i < PyList_Size(material_indexes); i++)
	{
		
		long material_index = PyLong_AsLong(PyList_GetItem(material_indexes, i));

		long vertex_index_0 = PyLong_AsLong(PyList_GetItem(vertex_indexes, 3*i));
		long vertex_index_1 = PyLong_AsLong(PyList_GetItem(vertex_indexes, 3*i+1));
		long vertex_index_2 = PyLong_AsLong(PyList_GetItem(vertex_indexes, 3*i+2));
		
		mat_index index0 = { vertex_index_0, material_index };
		mat_index index1 = { vertex_index_1, material_index };
		mat_index index2 = { vertex_index_2, material_index };
		
		toSort.push_back(index0);
		toSort.push_back(index1);
		toSort.push_back(index2);
	}

	std::sort(toSort.begin(), toSort.end(), [](const mat_index& a, const mat_index& b) {
		
		if (a.vertex_index == b.vertex_index) 
		{
			return a.mat_index < b.mat_index;
		}
		return a.vertex_index < b.vertex_index;
		});

	std::vector<mat_index> sorted;
	for (int i = 0; i < toSort.size() - 1; i++)
	{
		mat_index current = toSort[i];
		mat_index next = toSort[i+1];
		if((current.mat_index == next.mat_index) and (current.vertex_index == next.vertex_index))
		{
			continue;
		}
		sorted.push_back(current);
	}

	sorted.push_back(toSort.back());

	bool oneVertexMultipleMatIndexesError = false;

	long numberOfVertices = sorted.back().vertex_index + 1;

	PyObject* output_list = PyList_New(numberOfVertices);

	for (int i = 0; i < sorted.size() - 1; i++)
	{
		mat_index current = sorted[i];
		mat_index next = sorted[i + 1];
		
		if (current.vertex_index == next.vertex_index) 
		{
			oneVertexMultipleMatIndexesError = true;
			continue;
		}
		else
		{
			PyList_SetItem(output_list, current.vertex_index, PyLong_FromLong(current.mat_index));
		}
		
	}
	PyList_SetItem(output_list, sorted.back().vertex_index, PyLong_FromLong(sorted.back().mat_index));
	
	if (oneVertexMultipleMatIndexesError) 
	{
		std::cout << "SmoothieExporter: calculate_mat_indexes: Different materials have common vertices! You might get unexpected results!" << std::endl;
	}
	return output_list;
}