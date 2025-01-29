#include "Image.h"
#define _SMOOTHIE_ENGINE
#include "SmoothieCore.h"
#include "VMA.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void Image::destroyImage()
{
	vmaDestroyImage(VMA::getAllocator(), image, allocation);
	image = nullptr;
	
	vkDestroyImageView(SmoothieCore::getDevice(), imageView, nullptr);
	imageView = nullptr;

	vkDestroyBuffer(SmoothieCore::getDevice(), imageBuffer, nullptr);
	imageBuffer = nullptr;

}

VkSampler Samplers::Texture2DModelSampler = nullptr;
void Samplers::create()
{
	VkSamplerCreateInfo Texture2DModelSamplerInfo{};
	Texture2DModelSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	Texture2DModelSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	Texture2DModelSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	Texture2DModelSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	Texture2DModelSamplerInfo.minFilter = VK_FILTER_NEAREST;
	Texture2DModelSamplerInfo.magFilter = VK_FILTER_NEAREST;

	Texture2DModelSamplerInfo.unnormalizedCoordinates = false;

	vkCreateSampler(SmoothieCore::getDevice(), &Texture2DModelSamplerInfo, nullptr, &Texture2DModelSampler);
}

void Samplers::destroy()
{
	vkDestroySampler(SmoothieCore::getDevice(), Texture2DModelSampler, nullptr);
}


static VkCommandBuffer beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = SmoothieCore::getCommandPool();
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(SmoothieCore::getDevice(), &allocInfo, &commandBuffer);
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;

}

static void endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(SmoothieCore::getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(SmoothieCore::getGraphicsQueue());
	vkFreeCommandBuffers(SmoothieCore::getDevice(), SmoothieCore::getCommandPool(), 1, &commandBuffer);
}

static void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;


	VkPipelineStageFlags sourceStage = 0;
	VkPipelineStageFlags destinationStage = 0;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (
		oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}



	vkCmdPipelineBarrier(commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);


	endSingleTimeCommands(commandBuffer);


}

static void copyBufferToImage(
	VkBuffer buffer,
	VkImage image,
	uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(commandBuffer);
}

Texture2D::Texture2D(const std::string& file)
{
	filepath = file;

	//Loaded data
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(file.c_str(), &width, &height, &nOfChannels, STBI_rgb_alpha);
	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	
	createImage.extent.width = width;
	createImage.extent.height = height;
	createImage.extent.depth = 1;
	
	createImage.mipLevels = 1;
	createImage.arrayLayers = 1;
	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	createImage.format = VK_FORMAT_R8G8B8A8_SRGB;
	nOfChannels = 4;

	vkCreateImage(SmoothieCore::getDevice(), &createImage, nullptr, &image);

	VkBuffer stagingBuffer = nullptr;
	VmaAllocation stagingBufferAllocation = nullptr;

	VmaAllocationCreateInfo stagingBufferAllocInfo = {};
	stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	stagingBufferAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	stagingBufferAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	
	VkBufferCreateInfo stagingBufferCreateInfo{};
	stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferCreateInfo.size = width * height * nOfChannels;
	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	vmaCreateBuffer(VMA::getAllocator(), &stagingBufferCreateInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingBufferAllocation, nullptr);
	vmaCopyMemoryToAllocation(VMA::getAllocator(), data, stagingBufferAllocation, 0, width * height * nOfChannels);

	//Free data 
	if (data != nullptr)
	{
		stbi_image_free(data);
	}

	//Creating image
	VmaAllocationCreateInfo vmaImageAllocationInfo{};
	vmaImageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
	VmaAllocationInfo imageAllocationInfoDebug{};

	vmaCreateImage(VMA::getAllocator(), &createImage, &vmaImageAllocationInfo, &image, &allocation,&imageAllocationInfoDebug);

	transitionImageLayout(image, createImage.format, createImage.initialLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	copyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

	transitionImageLayout(image, createImage.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vmaDestroyBuffer(VMA::getAllocator(), stagingBuffer, stagingBufferAllocation);

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = image;
	imageViewInfo.format = createImage.format;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	vkCreateImageView(SmoothieCore::getDevice(), &imageViewInfo, nullptr, &imageView);

}
