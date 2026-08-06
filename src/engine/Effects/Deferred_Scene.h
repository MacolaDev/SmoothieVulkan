#pragma once
#include "Pipelines/Standard.h"
#include "../Core/Shader.h"
#include "../Core/Text.h"


#include "Models/Standard.h"

namespace Smoothie::DeferredRendering
 {

     class Deferred_Scene
     {
     public:
         explicit Deferred_Scene() {}
         explicit Deferred_Scene(unsigned int thread_count) {}

         int load_scene(const std::string &file);

     protected:

     };
 }
