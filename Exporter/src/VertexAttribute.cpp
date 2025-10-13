#include "VertexAttribute.h"


//int getVertexData(PyObject* listWithVertexData, VertexAttributeNamesPerType& namesPerType, std::vector<VertexAttributeBase*>& vertexAttributes)
//{
//    if (!PyList_Check(listWithVertexData))
//    {
//        PyErr_SetString(PyExc_AttributeError, "Input argument \"vertex_attributes\" must be a list!");
//        return -1;
//    }
//
//    const auto listSize = PyList_Size(listWithVertexData);
//    vertexAttributes.resize(listSize);
//
//    for (Py_ssize_t i = 0; i < listSize; i++)
//    {
//        PyObject* data = PyList_GetItem(listWithVertexData, i);
//
//        PyObject* attribute_name = PyObject_GetAttrString(data, "attribute_name");
//        PyObject* attribute_type = PyObject_GetAttrString(data, "attribute_type");
//        PyObject* attribute_data = PyObject_GetAttrString(data, "attribute_data");
//
//
//        if ((attribute_name == nullptr) ||
//            (attribute_type == nullptr) ||
//            (attribute_data == nullptr))
//        {
//            const std::string error = "vertex_attributes[" + std::to_string(i) + "]: Input element must have an attributes \"attribute_name\", \"attribute_type\" and \"attribute_data\"!";
//            PyErr_SetString(PyExc_AttributeError, error.c_str());
//            return -1;
//        }
//
//        const char* attributeName = PyUnicode_AsUTF8(attribute_name);
//        const char* attributeType = PyUnicode_AsUTF8(attribute_type);
//
//        int error = 0;
//
//        if (std::strcmp("INT", attributeType) == 0)
//        {
//            namesPerType.intNames.push_back(attributeName);
//            vertexAttributes[i] = new VertexAttributeInt(attributeName, attribute_data, error);
//
//            if (error != 0) return -1;
//            continue;
//        }
//
//        if (std::strcmp("FLOAT", attributeType) == 0)
//        {
//            namesPerType.floatNames.push_back(attributeName);
//            vertexAttributes[i] = new VertexAttributeFloat(attributeName, attribute_data, error);
//
//            if (error != 0) return -1;
//            continue;
//        }
//        
//        else if (std::strcmp("FLOAT2", attributeType) == 0)
//        {
//            namesPerType.vec2FloatNames.push_back(attributeName);
//            vertexAttributes[i] = new VertexAttributeVector2(attributeName, attribute_data, error);
//
//            if (error != 0) return -1;
//            continue;
//        }
//
//        else if (std::strcmp("FLOAT_VECTOR", attributeType) == 0)
//        {
//            namesPerType.vec3FloatNames.push_back(attributeName);
//            vertexAttributes[i] = new VertexAttributeVector3(attributeName, attribute_data, error);
//
//            if (error != 0) return -1;
//            continue;
//        }
//        
//
//        else if (std::strcmp("FLOAT_COLOR", attributeType) == 0)
//        {
//            namesPerType.vec4FloatNames.push_back(attributeName);
//            vertexAttributes[i] = new VertexAttributeVector4(attributeName, attribute_data, error);
//
//            if (error != 0) return -1;
//            continue;
//        }
//
//        else if (std::strcmp("QUATERNION", attributeType) == 0)
//        {
//            namesPerType.vec4FloatNames.push_back(attributeName);
//            vertexAttributes[i] = new VertexAttributeVector4(attributeName, attribute_data, error);
//
//            if (error != 0) return -1;
//            continue;
//        }
//
//        else 
//        {
//            vertexAttributes[i] = new VertexAttributeBase(attributeName);
//            continue;
//        }
//
//
//    }
//
//    return 0;
//}
//
//void freeVertexAttributeData(std::vector<VertexAttributeBase*>& data)
//{
//    for (size_t i = 0; i < data.size(); i++)
//    {
//        if (data[i] != nullptr)
//        {
//            delete data[i];
//            data[i] = nullptr;
//        }
//    }
//}
//
//
//VertexAttributeFloat::VertexAttributeFloat(const std::string& name, PyObject* pythonData, int& error) :VertexAttributeBase(name)
//{
//	const auto listSize = PyList_GET_SIZE(pythonData);
//	data.resize(listSize);
//	for (Py_ssize_t i = 0; i < listSize; i++)
//	{
//		data[i] = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pythonData, i)));
//	}
//
//}
//
//void VertexAttributeFloat::writeToBuffer(size_t index, std::vector<char>& buffer) const
//{
//}
//
//VertexAttributeInt::VertexAttributeInt(const std::string& name, PyObject* pythonData, int& error) :VertexAttributeBase(name)
//{
//    const auto listSize = PyList_GET_SIZE(pythonData);
//    data.resize(listSize);
//    for (Py_ssize_t i = 0; i < listSize; i++)
//    {
//        data[i] = static_cast<int>(PyLong_AsLong(PyList_GetItem(pythonData, i)));
//    }
//
//}
//
//void VertexAttributeInt::writeToBuffer(size_t index, std::vector<char>& buffer) const
//{
//}
//
//VertexAttributeInt8::VertexAttributeInt8(const std::string& name, PyObject* pythonData, int& error) :VertexAttributeBase(name)
//{
//    const auto listSize = PyList_GET_SIZE(pythonData);
//    data.resize(listSize);
//    for (Py_ssize_t i = 0; i < listSize; i++)
//    {
//        data[i] = static_cast<char>(PyLong_AsLong(PyList_GetItem(pythonData, i)));
//    }
//}
//
//void VertexAttributeInt8::writeToBuffer(size_t index, std::vector<char>& buffer) const
//{
//}
//
//VertexAttributeVector2::VertexAttributeVector2(const std::string& name, PyObject* pythonData, int& error) :VertexAttributeBase(name)
//{
//    const auto listSize = PyList_GET_SIZE(pythonData);
//    data.resize(listSize / 2);
//    for (Py_ssize_t i = 0; i < (listSize / 2); i++)
//    {
//       const float x = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pythonData, 2*i)));
//       const float y = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pythonData, 2*i+1)));
//       data[i] = SmoothieMath::Vector2(x, y);
//    }
//}
//
//void VertexAttributeVector2::writeToBuffer(size_t index, std::vector<char>& buffer) const
//{
//
//}
//
//VertexAttributeVector3::VertexAttributeVector3(const std::string& name, PyObject* pythonData, int& error) :VertexAttributeBase(name)
//{
//    const auto listSize = PyList_GET_SIZE(pythonData);
//    data.resize(listSize / 3);
//    for (Py_ssize_t i = 0; i < (listSize / 3); i++)
//    {
//        const float x = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pythonData, 3 * i + 0)));
//        const float y = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pythonData, 3 * i + 1)));
//        const float z = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pythonData, 3 * i + 2)));
//        data[i] = SmoothieMath::Vector3(x, y, z);
//    }
//}
//
//void VertexAttributeVector3::writeToBuffer(size_t index, std::vector<char>& buffer) const
//{
//}
//
//VertexAttributeVector4::VertexAttributeVector4(const std::string& name, PyObject* pythonData, int& error) :VertexAttributeBase(name)
//{
//    const auto listSize = PyList_GET_SIZE(pythonData);
//    data.resize(listSize / 4);
//    for (Py_ssize_t i = 0; i < (listSize / 4); i++)
//    {
//        const float x = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pythonData, 4 * i + 0)));
//        const float y = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pythonData, 4 * i + 1)));
//        const float z = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pythonData, 4 * i + 2)));
//        const float w = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(pythonData, 4 * i + 3)));
//        data[i] = SmoothieMath::Vector4(x, y, z, w);
//    }
//   
//}
//
//void VertexAttributeVector4::writeToBuffer(size_t index, std::vector<char>& buffer) const
//{
//}
