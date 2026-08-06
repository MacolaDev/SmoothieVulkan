//
// Created by macola on 4/23/26.
//
#include "Textures.h"

#include "Core/SmoothieCore.h"

int Smoothie::DeferredRendering::Texture2D_Attachment::create()
{

    VkImageCreateInfo _createImage{};
    _createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    _createImage.imageType = VK_IMAGE_TYPE_2D;
    _createImage.extent = m_Extent;
    _createImage.mipLevels = 1;
    _createImage.arrayLayers = 1;
    _createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
    _createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _createImage.usage = m_ImageUsage;
    _createImage.samples = VK_SAMPLE_COUNT_1_BIT;
    _createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    _createImage.format = m_ImageFormat;

    VmaAllocationCreateInfo _allocInfo = {};
    _allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    _allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    _allocInfo.priority = 1.0f;

    if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_createImage, &_allocInfo, &m_Image, &m_Allocation, nullptr) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create image.");
        return 1;
    }

    VkImageViewCreateInfo _createImageView{};
    _createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    _createImageView.image = m_Image;
    _createImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    _createImageView.format = m_ImageFormat;
    _createImageView.subresourceRange.aspectMask = m_ImageAspect;
    _createImageView.subresourceRange.levelCount = 1;
    _createImageView.subresourceRange.layerCount = 1;
    if (vkCreateImageView(SmoothieCore::getDevice(), &_createImageView, nullptr, &m_ImageView) != VK_SUCCESS)
    {
        SMOOTHIE_ERROR_WITH_SRC("Failed to create image view.");
        return 1;
    }
    return 0;
}

void Smoothie::DeferredRendering::Texture2D_Attachment::destroy()
{
    if (m_ImageView != nullptr)
    {
        vkDestroyImageView(SmoothieCore::getDevice(), m_ImageView, nullptr);
        m_ImageView = nullptr;
    }

    if (m_Image != nullptr)
    {
        vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_Image, m_Allocation);
        m_Image = nullptr;
        m_Allocation = nullptr;
    }

    m_Extent = {};
    m_ImageFormat = VK_FORMAT_UNDEFINED;
    m_ImageUsage = 0;
    m_ImageAspect = 0;
}
