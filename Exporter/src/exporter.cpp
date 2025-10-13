#define PY_SSIZE_T_CLEAN
#include "exporter.h"
#include <iostream>


static PyModuleDef SEModuleDefinition = 
{
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