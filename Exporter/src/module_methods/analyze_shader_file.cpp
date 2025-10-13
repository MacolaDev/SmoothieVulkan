#include "exporter.h"
#include <filesystem>
#include <ResourceManager/ShaderFile.h>

PyObject* analyze_shader_file(PyObject* self, PyObject* args) 
{
	PyObject* shaderFilepathPython = PyObject_GetAttrString(args, "shader_filepath");
	if (shaderFilepathPython == nullptr) 
	{
		PyErr_SetString(PyExc_AttributeError, "Input argument does not have a \"shader_filepath\" attribute!");
		return nullptr;
	};
	
	PyObject* vertexShaderInputPython = PyObject_GetAttrString(args, "vertex_shader_input");
	if (vertexShaderInputPython == nullptr)
	{
		PyErr_SetString(PyExc_AttributeError, "Input argument does not have a \"vertex_shader_input\" attribute!");
		return nullptr;
	};

	PyObject* uniformDataInputPython = PyObject_GetAttrString(args, "uniform_data_input");
	if (uniformDataInputPython == nullptr)
	{
		PyErr_SetString(PyExc_AttributeError, "Input argument does not have a \"uniform_data_input\" attribute!");
		return nullptr;
	};

	const char* shaderFilepath = PyUnicode_AsUTF8(shaderFilepathPython);
	if (!std::filesystem::exists(shaderFilepath)) return Py_False;
	//const auto& shaderFile = ShaderFile(shaderFilepath);
	
	//Analyzing model descriptor data
	//for (const auto& descriptor: shaderFile.modelDescriptorData)
	//{
	//	PyObject* dataTuple = PyTuple_New(2); //Name and type
	//	if (dataTuple == nullptr) return nullptr;

	//	PyTuple_SetItem(dataTuple, 0, PyLong_FromSize_t(descriptor.type));
	//	PyTuple_SetItem(dataTuple, 1, PyUnicode_FromString(descriptor.name.c_str()));
	//	PyList_Append(uniformDataInputPython, dataTuple);
	//	//Py_CLEAR(dataTuple);
	//}
	//
	//for (const auto& vertexData : shaderFile.vertexShaderInputData)
	//{
	//	PyObject* dataTuple = PyTuple_New(3); //Name and type
	//	if (dataTuple == nullptr) return nullptr;

	//	PyTuple_SetItem(dataTuple, 0, PyLong_FromSize_t(vertexData.location));
	//	PyTuple_SetItem(dataTuple, 1, PyLong_FromSize_t(vertexData.type));
	//	PyTuple_SetItem(dataTuple, 2, PyUnicode_FromString(vertexData.name.c_str()));
	//	PyList_Append(vertexShaderInputPython, dataTuple);
	//	//Py_CLEAR(dataTuple);
	//}



	Py_INCREF(Py_None);
	return Py_None;
}