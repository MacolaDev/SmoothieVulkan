#pragma once
#include "Core/Common.h"

#include "../Models/Standard.h"

namespace Smoothie
{
    namespace DeferredRendering
    {

        //CP -> Compute Pass
        //GP -> Graphics Pass
        struct Pipeline_CommandBuffers
        {
            //Frustum culling buffer
            VkCommandBuffer CP_Culling = nullptr;
            VkCommandBuffer CP_Compute = nullptr;
            VkCommandBuffer GP_gBuffer = nullptr;
            VkCommandBuffer GP_Shadow = nullptr;
            VkCommandBuffer CP_SS_Effects = nullptr;
            VkCommandBuffer GP_HDR = nullptr;
            VkCommandBuffer CP_Post_processing = nullptr;
            VkCommandBuffer GP_Present = nullptr;

            VkCommandPool _command_pool = nullptr;
        };

        struct Pipeline_Data
        {
            Pipeline_CommandBuffers buffers;
            VkDescriptorSet descriptorSet = nullptr;
            VkViewport viewport{};
            VkRect2D scissor{};
            unsigned int imageIndex = 0;
            unsigned int frameIndex = 0;
        };


        class Pipeline_Base
        {
        public:

            virtual int create(const std::string& shader) = 0;
            virtual int update() {return 0;}
            virtual void destroy() = 0;

            virtual void record_data(const Pipeline_Data& drawData) const = 0;


            virtual ~Pipeline_Base() = default;
        };


    }
}