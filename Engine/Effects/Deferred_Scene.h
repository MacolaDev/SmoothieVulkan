#pragma once
#include "Deferred_Pipeline.h"
#include "Core/Scene.h"
#include "Core/Shader.h"

#include "ResourceManager/XML.h"

#include "Effects/Deferred_Model.h"

 namespace Smoothie::DeferredRendering
 {

     struct LoadableObject_Model: public Smoothie::LoadableObject_Base
     {
         int create() override {return 0;};

         void destroy() override {};

         int recreate() override { return 0; };


         std::shared_ptr<Standard_Model> model;

         std::string shaderFile;
         std::string modelFile;
         std::string geometryFile;
         SmoothieMath::Vector3 bbMin;
         SmoothieMath::Vector3 bbMax;
         std::vector<XML::Element> modelProperties;

     };


     //Pipelines for standard models
     struct LoadableObject_Shader: public Smoothie::LoadableObject_Base
     {
         int create() override;
         void destroy() override;
         int recreate() override;

         Smoothie::ShaderFile shader;
     };


     class Deferred_Scene: public Smoothie::Scene_Base
     {
     public:
         int load_scene(const std::string &file) override;

         void load_model(const Smoothie::XML::Element& element);


     protected:

     };
 }
