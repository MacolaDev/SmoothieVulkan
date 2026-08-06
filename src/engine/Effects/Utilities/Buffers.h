#pragma once
#include "Common.h"
#include "Core/Buffer.h"


namespace Smoothie
{
    namespace DeferredRendering
    {

        class Buffer_Uniform: public Buffer_Base
        {
            VkBufferUsageFlags m_Usage = 0;

            int create() override;
        public:

            inline int create(VkBufferUsageFlags usage, VkDeviceSize size) {m_Usage = usage; m_BufferSize = size; return create();}
            void destroy() override;

        };


        class Buffer_MappedUniform: public Buffer_Base
        {
            std::vector<char> m_Data;
            void* m_pMappedData = nullptr;

            int create() override;
        public:

            inline int create(VkDeviceSize size) {m_BufferSize = size; return create();}
            void destroy() override;


            int map();

            inline void copy_data(const std::vector<char>& data) {m_Data = data;}
            void copy_to_gpu(VkCommandBuffer transferBuffer) const;

            void unmap();

        };


    }
}
