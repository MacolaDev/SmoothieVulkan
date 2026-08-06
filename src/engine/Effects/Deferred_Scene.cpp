//
// Created by macola on 1/29/26.
//

#include "Deferred_Scene.h"
#include "Core/SmoothieCore.h"

using namespace Smoothie;

int Smoothie::DeferredRendering::Deferred_Scene::load_scene(const std::string &file)
{
    TextFile _scene_file;
    if (_scene_file.load(file) != 0)
    {
        SmoothieCore::logError("Failed to load scene file: " + file);
        return 1;
    }
    

    // Element rootElement;
    // XMLError error;
    // if (build_tree_from_xml_file(file, rootElement, error))
    // {
    //     std::cout << "Failed to parse scene xml file!" << std::endl;
    //     return 1;
    // }
    //
    // //Skybox
    // const Element &skyboxElement = rootElement.getChild("skybox");
    // //read_skybox(skyboxElement);
    //
    // //Models
    // const Element &modelsElement = rootElement.getChild("models");
    //
    // const std::vector<Element> &models = modelsElement.children;
    //for (const auto &_model: models)
    {
        //SmoothieCore::Submit_ExecutionThread(std::async(std::launch::async, &Deferred_Scene::load_model, this, _model));
    }

    return 0;
}


// static std::mutex g_loadingMutex_Shaders;
// void Smoothie::DeferredRendering::Deferred_Scene::load_model(const Element &_model_element)
// {
//     const auto &matrix = _model_element.getChild("matrix");
//
//     SmoothieMath::Matrix4x4 _modelMatrix;
//     if (_model_element.hasChild("position") && _model_element.hasChild("scale") && _model_element.hasChild("rotation"))
//     {
//         const auto &position = _model_element.getChild("position");
//         const auto &scale = _model_element.getChild("scale");
//         const auto &rotation = _model_element.getChild("rotation");
//         _modelMatrix.transformMatrix(position.getVector3(), rotation.getVector3(), scale.getVector3());
//     }
//
//     const auto &_model_file = _model_element.getChild("file").textContent;
//     if (!std::filesystem::exists(_model_file))
//     {
//         std::cout << "Model file: " << _model_file << " does not exist!" << std::endl;
//         return;
//     }
//
//
//     //If model exists, add new model matrix.
//     // if (const auto _obj_ptr = get_Object(_model_file); _obj_ptr != nullptr)
//     // {
//     //     auto *_model_scene_object_ptr = dynamic_cast<LoadableObject_Model *>(_obj_ptr.get());
//     //     assert(_model_scene_object_ptr != nullptr);
//     //     if (_model_scene_object_ptr == nullptr) return;
//     //
//     //
//     //     if (_model_scene_object_ptr->model == nullptr) return;
//     //
//     //     auto *_model_ptr = _model_scene_object_ptr->model.get();
//     //     if (_model_ptr == nullptr) return;
//     //     _model_ptr->add_model_matrix(_modelMatrix);
//     //     return;
//     // }
//
//     XML::Element _modelRootElement;
//     XMLError _modelError;
//     if (XML::build_tree_from_xml_file(_model_file, _modelRootElement, _modelError) != 0)
//     {
//         std::cout << "Failed to parse scene model xml file!" << std::endl;
//         return;
//     }
//
//
//
//
//     DeferredRendering::Model_Standard _model;
//     // for (const auto &_child : _modelRootElement.children)
//     // {
//     //     //Get shader/pipeline
//     //     if (_child.name == "shader")
//     //     {
//     //         std::lock_guard<std::mutex> _lock(g_loadingMutex_Shaders);
//     //         auto& _shader_file = _child.textContent;
//     //         if (const auto& _obj = std::dynamic_pointer_cast<LoadableObject_Shader>(get_Object(_shader_file)); _obj != nullptr)
//     //         {
//     //             if (_obj->shader.get_Filepath().empty() == true && _obj->shader.create(_shader_file) != 0)
//     //             {
//     //                 std::cout << "Failed to create shader!" << std::endl;
//     //             }
//     //             continue;
//     //         }
//     //
//     //         auto _new_obj = std::make_shared<LoadableObject_Shader>();
//     //         if (_new_obj->shader.create(_shader_file) != 0)
//     //         {
//     //             std::cout << "Failed to create shader!" << std::endl;
//     //             continue;
//     //         }
//     //         add_Object(_shader_file, _new_obj);
//     //     }
//     // }
//
//
//
//     _model.add_model_matrix(_modelMatrix);
//     //_model.setModelFile(_model_element.getChild("file").textContent);
//     // if (_model.create() != 0)
//     // {
//     //     std::cout << "Failed to create model " << _model.getModelFile() << std::endl;
//     // }
// }
