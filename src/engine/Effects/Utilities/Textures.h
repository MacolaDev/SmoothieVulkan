#pragma once
#include "Common.h"
#include "Core/Texture.h"

namespace Smoothie
{
    namespace DeferredRendering
    {

        class Texture2D_Attachment: public Texture_Base
        {
            VkExtent3D m_Extent = {};
            VkFormat m_ImageFormat = VK_FORMAT_UNDEFINED;
            VkImageUsageFlags m_ImageUsage = 0;
            VkImageAspectFlags m_ImageAspect = 0;
        protected:
            int create() override;
        public:


            inline int create(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect)
            {
                m_Extent = extent;
                m_ImageFormat = format;
                m_ImageUsage = usage;
                m_ImageAspect = aspect;
                return create();
            }

            inline VkFormat getImageFormat() {return m_ImageFormat;}

            void destroy() override;

        };

    }

}