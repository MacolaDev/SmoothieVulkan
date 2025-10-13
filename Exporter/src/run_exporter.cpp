#include "exporter.h"
#include <string>
#include <iostream>
#include <filesystem>

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ResourceManager/XML.h"

#include "ShaderFile.h"
#include "Property.h"
#include "VertexAttribute.h"

using namespace Smoothie::XML;
using namespace SmoothieExporter;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static std::vector<PropertyBase*> getModelProperties(
    const std::vector<ShaderVariable>& data,
    const std::vector<const char*> texturePaths2D, const Element& propertiesElement) 
{
    std::vector<PropertyBase*> output(data.size());
    
    for (size_t i = 0; i < data.size(); i++)
    {
        const std::string& propertyName = data[i].name;
        switch (data[i].type)
        {
        case MODEL_DESCRIPTOR_DATA_TYPE_UNSIGNED_INT:
            if (propertyName == "ModelID") 
            {
                output[i] = nullptr;
                break;
            }

            output[i] = new PropertyUnsignedInt(propertyName);
            output[i]->set_selected_option(propertiesElement);
            break;

        case MODEL_DESCRIPTOR_DATA_TYPE_FLOAT:
            output[i] = new PropertyFloat(propertyName);
            output[i]->set_selected_option(propertiesElement);
            break;

        case MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_VEC2:
            output[i] = new PropertyVector2(propertyName);
            output[i]->set_selected_option(propertiesElement);
            break;

        case MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_VEC3:
            output[i] = new PropertyVector3(propertyName);
            output[i]->set_selected_option(propertiesElement);
            break;

        case MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_VEC4:
            output[i] = new PropertyVector4(propertyName);
            output[i]->set_selected_option(propertiesElement);
            break;



        case MODEL_DESCRIPTOR_SAMPLER_2D:
            output[i] = new PropertySampler2D(propertyName, texturePaths2D);
            output[i]->set_selected_option(propertiesElement);
            break;


        default:
            output[i] = nullptr;
            break;
        }
    }
 
    return output;
}

static void clearProperties(std::vector<PropertyBase*> data) 
{
    for (auto& d : data) 
    {
        delete d;
    }
}

static int getTextures(PyObject* listWithTextures, std::vector<const char*>& textureData) 
{
    if (!PyList_Check(listWithTextures))
    {
        PyErr_SetString(PyExc_AttributeError, "Input argument \"saved_textures\" must be a list!");
        return -1;
    }
    
    const auto listSize = PyList_Size(listWithTextures);
    textureData.resize(listSize);
    for (Py_ssize_t i = 0; i < listSize; i++)
    {
        textureData[i] = PyUnicode_AsUTF8(PyList_GetItem(listWithTextures, i));
    }

    return 0;
}

struct SelectVertexAttribute
{
    std::string attributeName;
    std::vector<const char*> validVertexAttributeNames;
    int selected = 0;
};

static int createVertexAttributeNames(
    const std::vector<ShaderPipeInput>& inputData,
    const VertexAttributeNamesPerType& attributeNames, 
    std::vector<SelectVertexAttribute>& selectableAttributes,
    int& isExportPossible)
{
    selectableAttributes.resize(inputData.size());
    for (size_t i = 0; i < selectableAttributes.size(); i++)
    {
        selectableAttributes[i].attributeName = inputData[i].name;

        switch (inputData[i].type)
        {
        case MODEL_DESCRIPTOR_DATA_TYPE_FLOAT:
            selectableAttributes[i].validVertexAttributeNames = attributeNames.floatNames;
            break;

        case MODEL_DESCRIPTOR_DATA_TYPE_INT:
            selectableAttributes[i].validVertexAttributeNames = attributeNames.intNames;
            break;

        case MODEL_DESCRIPTOR_DATA_TYPE_BOOL:
            selectableAttributes[i].validVertexAttributeNames = attributeNames.boolNames;
            break;

        case MODEL_DESCRIPTOR_DATA_TYPE_BYTE:
            selectableAttributes[i].validVertexAttributeNames = attributeNames.int8Names;
            break;

        case MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_VEC2:
            selectableAttributes[i].validVertexAttributeNames = attributeNames.vec2FloatNames;
            break;

        case MODEL_DESCRIPTOR_DATA_TYPE_INT_VEC2:
            selectableAttributes[i].validVertexAttributeNames = attributeNames.vec2IntNames;
            break;

        case MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_VEC3:
            selectableAttributes[i].validVertexAttributeNames = attributeNames.vec3FloatNames;
            break;

        case MODEL_DESCRIPTOR_DATA_TYPE_FLOAT_VEC4:
            selectableAttributes[i].validVertexAttributeNames = attributeNames.vec4FloatNames;
            break;

        default:
            isExportPossible &= 0;
            break;
        }

    }

    return 0;
}


static inline int startOpenGL(GLFWwindow** window) 
{
    if (!glfwInit())
    {
        PyErr_SetString(PyExc_RuntimeError, "run_exporter: Failed to initilize GLFW!");
        return -1;
    }

    glfwSetErrorCallback(glfw_error_callback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    *window = glfwCreateWindow(1280, 720, "Smoothie Exporter", nullptr, nullptr);
    if (*window == nullptr)
    {
        PyErr_SetString(PyExc_RuntimeError, "run_exporter: Can't create glfw window!");
        return -1;
    }

    glfwMakeContextCurrent(*window);
    glfwSwapInterval(1);
    return 0;
}

static int readPythonString(std::string& result, const char* attributeName, PyObject* args) 
{
    PyObject* pythonStringOject = PyObject_GetAttrString(args, attributeName);
    if (pythonStringOject == nullptr)
    {
        std::string error = "Input argument does not have a \"" + std::string(attributeName) + "\" attribute!";
        PyErr_SetString(PyExc_AttributeError, error.c_str());
        return -1;
    };
    result = PyUnicode_AsUTF8(pythonStringOject);
    return 0;
}

static int getPythonAttribute(PyObject** result, const char* attributeName, PyObject* args) 
{
    if (result == nullptr) return -1;

    *result = PyObject_GetAttrString(args, attributeName);
    if (*result == nullptr)
    {
        std::string error = "Input argument does not have a \"" + std::string(attributeName) + "\" attribute!";
        PyErr_SetString(PyExc_AttributeError, error.c_str());
        return -1;
    };
    return 0;
}

static bool isPathValid(const std::string& path) 
{
    const std::filesystem::path fspath = path;
    const auto dir = fspath.parent_path();
    if (std::filesystem::is_directory(dir)) return 1;
    return 0;
}

int getRelativeFromFullPath(const std::string& fullPath, std::string& relative_path, const std::string& key);

static int write_model_file(
    const std::string& modelFilePath, 
    const std::string& shaderFile,
    const std::string& geometryFile,
    const std::vector<PropertyBase*>& properties,
    const std::string& bbMin, const std::string& bbMax)
{
    Element rootElement("model");

    //Geometry file
    std::string relative_geometryFile;
    if (getRelativeFromFullPath(geometryFile, relative_geometryFile, "resources") != 0) return -1;
    rootElement.children.push_back({ "geometryFile", relative_geometryFile });

    //Shader file
    std::string relative_shaderFile;
    if (getRelativeFromFullPath(shaderFile, relative_shaderFile, "shaders") != 0) return -1;
    rootElement.children.push_back({ "shader", relative_shaderFile });

    //Shader properties
    Element properties_element("property");
    for (const PropertyBase* _property : properties) 
    {
        if (_property != nullptr) properties_element.children.push_back(_property->as_xml_element());
    }
    rootElement.children.push_back(properties_element);


    //Bounding box
    Element boundingBoxElement("BoundingBox");
    boundingBoxElement.children.push_back({ "min", bbMin });
    boundingBoxElement.children.push_back({ "max", bbMax });
    rootElement.children.push_back(boundingBoxElement);

    XMLError error;
    return build_xml_file_from_tree(rootElement, modelFilePath, error);
}

PyObject* run_exporter(PyObject* self, PyObject* args) 
{
    //Model save filepath
    std::string modelSaveFilepath;
    if(readPythonString(modelSaveFilepath, "model_save_filepath", args) != 0) return nullptr;
    int isModelFileExportPossible = isPathValid(modelSaveFilepath);

    /*std::string modelName;
    if (readPythonString(modelName, "model_name", args) != 0) return nullptr;*/
    
    /*std::string geometryFile;
    if (readPythonString(geometryFile, "geometry_save_filepath", args) != 0) return nullptr;*/

    std::string bbMin, bbMax;
    if (readPythonString(bbMin, "bbMin", args) != 0) return nullptr;
    if (readPythonString(bbMax, "bbMax", args) != 0) return nullptr;

    //***************************** get saved data from belnder ************************************//
    std::string modelData;
    if (readPythonString(modelData, "model_data_str", args) != 0) return nullptr;
    
    Element rootElement;
    XMLError error;
    if (built_tree_from_string(modelData, rootElement, error) != 0) 
    {
        rootElement.name = "root";
    };
    

    //***************************** Get textures from blender *************************************//
    PyObject* savedTexturesPython = nullptr;
    if (getPythonAttribute(&savedTexturesPython, "saved_textures", args) != 0) return nullptr;

    std::vector<const char*> textures2D;
    if (getTextures(savedTexturesPython, textures2D) != 0) return nullptr;


    //***************************** Get shader file from blender ************************************//
    std::string modelShader;
    if (readPythonString(modelShader, "selected_shader", args) != 0) return nullptr;

    ShaderFileMap shaderFile;
    if (parse_shader_file(modelShader, shaderFile) != 0)
    {
        std::string _error = "Failed to parse shader file!";
        PyErr_SetString(PyExc_AttributeError, _error.c_str());
        return nullptr;
    }

    std::vector<ShaderStage> __vertex_shaders;
    for (const auto& [__name, __shader] : shaderFile)
    {
        if (__name.stage == VK_SHADER_STAGE_VERTEX_BIT) __vertex_shaders.push_back(__shader);
    }


    //std::vector<PropertyBase*> modelProperties = getModelProperties(shaderFile, textures2D, rootElement.getChild("selected-model-properties"));
    //
    //
    ////******************************* Get vertex data from blender ************************************//
    //PyObject* vertexAttributesPython = nullptr;
    //if (getPythonAttribute(&vertexAttributesPython, "vertex_attributes", args) != 0) return nullptr;

    //VertexAttributeNamesPerType vertexAttributeNamesPerType;
    //std::vector<VertexAttributeBase*> vertexAttributes;
    //if (getVertexData(vertexAttributesPython, vertexAttributeNamesPerType, vertexAttributes) != 0) return nullptr;
    //
    //std::vector<SelectVertexAttribute> selectableVertexAttributes;
    //int isGeometryFileExportPossible = 1;
    //if (createVertexAttributeNames(shaderFile, vertexAttributeNamesPerType, selectableVertexAttributes, isGeometryFileExportPossible) != 0)
    //{
    //    return nullptr;
    //}


    //******************************* Starting GLFW stuff *********************************************//
    GLFWwindow* window = nullptr;
    if(startOpenGL(&window) != 0 ) return nullptr;
    

    //************************ ImGui stuff ********************************//
	IMGUI_CHECKVERSION();
	auto imGuiContext = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450");

    //main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();
        
        ImGui::SetNextWindowPos({ 0, 0 });
        const ImVec2 windowSizeFloat = { static_cast<float>(display_w), static_cast<float>(display_h) };
        ImGui::SetNextWindowSize(windowSizeFloat);
        ImGuiWindowFlags windowFlags = 0;
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        
        ImGui::Begin("MAIN_WINDOW", nullptr, windowFlags);

        
        //ImGui::SeparatorText("General data");
        //
        //ImGui::Text("Selected object: ");
        //ImGui::SameLine();
        //ImGui::Text(modelName.c_str());

        //ImGui::Text("Selected shader: ");
        //ImGui::SameLine();
        //ImGui::Text(modelShader.c_str());
        //

        ////Geometry part
        //ImGui::SeparatorText("Vertex buffer data");
        //for (auto& attribute : selectableVertexAttributes)
        //{
        //    ImGui::Combo(
        //        attribute.attributeName.c_str(), 
        //        &attribute.selected,
        //        attribute.validVertexAttributeNames.data(), 
        //        static_cast<int>(attribute.validVertexAttributeNames.size()));
        //}

        //ImGui::Text("Output geometry file: ");
        //ImGui::SameLine();
        //ImGui::Text(geometryFile.c_str());

        //ImGui::BeginDisabled(isGeometryFileExportPossible == 0);
        //ImGui::Button("Export geometry file!");
        //ImGui::EndDisabled();

        //if (isGeometryFileExportPossible == 0) ImGui::SameLine(), ImGui::Text("Some values are not provided! Can't export geometry file!");

        ////Model properties part
        //ImGui::SeparatorText("Model properties");
        //for (auto& property: modelProperties)
        //{
        //    if (property != nullptr) property->draw();
        //}

        //ImGui::Text("Output model file: ");
        //ImGui::SameLine();
        //ImGui::Text(modelSaveFilepath.c_str());
        //ImGui::BeginDisabled(isModelFileExportPossible == 0);
        //if (ImGui::Button("Export model file!"))
        //{
        //    int result = write_model_file(modelSaveFilepath, modelShader, geometryFile, modelProperties, bbMin, bbMax);
        //    if (result == 0)
        //    {
        //        ImGui::Text("Exported with no errors! :D");
        //    }

        //}
        //ImGui::EndDisabled();
        //if (isModelFileExportPossible == 0) ImGui::SameLine(), ImGui::Text("Target path is not valid!");

        ImGui::End();

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

    }


    rootElement.children.clear();

    ////Saving properties data
    //Element selected_model_properties("selected-model-properties");
    //for (size_t i = 0; i < modelProperties.size(); i++)
    //{
    //    if (modelProperties[i] != nullptr) modelProperties[i]->save_selected_option(selected_model_properties);
    //}
    //rootElement.children.push_back(selected_model_properties);

    //clearProperties(modelProperties);
    //freeVertexAttributeData(vertexAttributes);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    std::string data_for_saving;
    if (build_string_from_tree(rootElement, data_for_saving, error) != 0) data_for_saving = modelData;
    return PyUnicode_FromString(data_for_saving.c_str());
}