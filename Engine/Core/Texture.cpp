#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Core/Constants.h"
#include "Core/SmoothieCore.h"
#include "Core/Multithreading.h"
#include <iostream>

using namespace Smoothie;

static std::mutex loadMutex;
static int loadImageSTB
(
	VkImage& image,
	VkImageView& imageView,
	VmaAllocation& allocation,
	const std::string& path
)
{
	int width, height;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path.c_str(), &width, &height, nullptr, STBI_rgb_alpha);
	if (data == nullptr)
	{
		std::cout << "Failed to load image: " << path << std::endl;
		return 1;
	}
	std::lock_guard<std::mutex> lock(loadMutex);

	auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	if (mipLevels > SMOOTHIE_IMAGE_MAX_LOD_MODEL_IMAGE) mipLevels = SMOOTHIE_IMAGE_MAX_LOD_MODEL_IMAGE;

	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	createImage.extent.width = width;
	createImage.extent.height = height;
	createImage.extent.depth = 1;
	createImage.mipLevels = mipLevels;
	createImage.arrayLayers = 1;
	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createImage.format = VK_FORMAT_R8G8B8A8_UNORM;

	//Creating image
	VmaAllocationCreateInfo vmaImageAllocationInfo{};
	vmaImageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &createImage, &vmaImageAllocationInfo, &image, &allocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create image!" << std::endl;
		return 1;
	}


	std::string imageAllocationName = "Texture: " + path;
	vmaSetAllocationName(SmoothieCore::getVulkanMemoryAllocator(), allocation, imageAllocationName.c_str());


	VkBuffer stagingBuffer = nullptr;
	VmaAllocation stagingBufferAllocation = nullptr;

	VmaAllocationCreateInfo stagingBufferAllocInfo = {};
	stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	stagingBufferAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	stagingBufferAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	VkBufferCreateInfo stagingBufferCreateInfo{};
	stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferCreateInfo.size = width * height * 4;
	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &stagingBufferCreateInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingBufferAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create staging buffer" << std::endl;
		return 1;
	}


	const std::string stagingBufferAllocationName = "Texture stageing buffer: " + path;
	vmaSetAllocationName(SmoothieCore::getVulkanMemoryAllocator(), stagingBufferAllocation, stagingBufferAllocationName.c_str());

	if (vmaCopyMemoryToAllocation(SmoothieCore::getVulkanMemoryAllocator(), data, stagingBufferAllocation, 0, width * height * 4) != VK_SUCCESS)
	{
		std::cout << "Failed to copy image data to staging buffer!" << std::endl;
		return 1;
	}

	//Free data 
	if (data != nullptr)
	{
		stbi_image_free(data);
	}

	// transitionImageLayout(image, createImage.format, createImage.initialLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	// copyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	// generateMipmaps(image, static_cast<uint32_t>(width), static_cast<uint32_t>(height), mipLevels);
	//transitionImageLayout(image.image, createImage.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);


	vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), stagingBuffer, stagingBufferAllocation);
	
	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = image;
	imageViewInfo.format = createImage.format;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = mipLevels;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	if (vkCreateImageView(SmoothieCore::getDevice(), &imageViewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		std::cout << "Failed to create image view" << std::endl;
		return 1;
	}

	return 0;
}


static std::mutex mutex;
int Texture2D::create(const std::string& file)
{
	filepath = file;
	std::lock_guard<std::mutex> lock(mutex);
	if (loadImageSTB(image, imageView, allocation, file) != 0)
	{
		std::cout << "Failed to load texture " << file << std::endl;
		return 1;
	}
	
	return 0;
}

void Texture2D::destroy()
{
	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), image, allocation);
	image = nullptr;
	allocation = nullptr;

	vkDestroyImageView(SmoothieCore::getDevice(), imageView, nullptr);
	imageView = nullptr;
}

int Smoothie::DefaultTexture2D::create()
{
	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	createImage.extent.width = 256;
	createImage.extent.height = 256;
	createImage.extent.depth = 1;
	createImage.mipLevels = 1;
	createImage.arrayLayers = 1;
	createImage.tiling = VK_IMAGE_TILING_LINEAR;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createImage.format = VK_FORMAT_R8G8B8A8_UNORM;

	VmaAllocationCreateInfo vmaImageAllocationInfo{};
	vmaImageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &createImage, &vmaImageAllocationInfo, &image, &allocation, nullptr) != VK_SUCCESS)
	{

		return 1;
	}

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = image;
	imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	if (vkCreateImageView(SmoothieCore::getDevice(), &imageViewInfo, nullptr, &imageView) != VK_SUCCESS)
	{

		return 1;
	}

	ImmediateCommandBuffer _gpuWorkData;
	if (_gpuWorkData.create() != 0)
	{
		std::cout << "Failed to create GPU work data" << std::endl;
		return 1;
	}
	_gpuWorkData.begin();
	VkCommandBuffer _commandBuffer = _gpuWorkData.get_CommandBuffer();

	VkImageSubresourceRange _imageSubresourseRange{};
	_imageSubresourseRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	_imageSubresourseRange.baseMipLevel = 0;
	_imageSubresourseRange.levelCount = 1;
	_imageSubresourseRange.baseArrayLayer = 0;
	_imageSubresourseRange.layerCount = 1;
	

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = createImage.initialLayout;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange = _imageSubresourseRange;

	vkCmdPipelineBarrier(_commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);


	VkClearColorValue _clearColorValue{};
	_clearColorValue.float32[0] = 0.69f;
	_clearColorValue.float32[1] = 0.69f;
	_clearColorValue.float32[2] = 0.69f;
	_clearColorValue.float32[3] = 1.0f;

	vkCmdClearColorImage(_commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &_clearColorValue, 1, &_imageSubresourseRange);
	
	barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	vkCmdPipelineBarrier(_commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);


	_gpuWorkData.end();
	_gpuWorkData.submit();
	_gpuWorkData.destroy();

	return 0;
}

void Smoothie::DefaultTexture2D::destroy()
{
	vkDestroyImageView(SmoothieCore::getDevice(), imageView, nullptr);
	imageView = nullptr;

	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), image, allocation);
	image = nullptr, allocation = nullptr;

}
