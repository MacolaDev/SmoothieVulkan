#define PY_SSIZE_T_CLEAN
#include "exporter.h"
#include <iostream>

static PyMethodDef SEModuleDefs[] =
{
    {"calculate_uv_data", calculate_uv_data, METH_O, calculate_uv_data_docs},
    {"calculate_tangent_data", calculate_tangent_data, METH_O, calculate_tangent_data_docs},
    {"write_geometry_file", write_geometry_file, METH_O, write_geometry_file_data_docs},
    {"calculate_uv_data_averaged", calculate_uv_data_averaged, METH_O, calculate_uv_data_averaged_docs},
    {"calculate_mat_indexes", calculate_mat_indexes, METH_O, calculate_mat_indexes_docs},
    {NULL, NULL, 0, NULL}
};


static PyModuleDef SEModuleDefinition = {
    PyModuleDef_HEAD_INIT,
    "SmoothieExporter",   /* name of module */
    "SmoothieExporter is a Python extention module, used with Blender's Python to process data from Blender for Smoothie render engine.",
    -1,
    SEModuleDefs
};

PyMODINIT_FUNC PyInit_SmoothieExporter(void)
{
    return PyModule_Create(&SEModuleDefinition);
}