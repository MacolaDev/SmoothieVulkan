// #include "Icons.h"
//
// static int load_icon(const std::string& path, VkImage& image, VkImageView& imageView, VmaAllocation& allocation)
// {
//
// 	int width, height;
// 	stbi_set_flip_vertically_on_load(false);
// 	unsigned char* data = stbi_load(path.c_str(), &width, &height, nullptr, STBI_rgb_alpha);
// 	if (data == nullptr)
// 	{
// 		//std::cout << "Failed to load file: " << path << std::endl;
// 		return 1;
// 	}
//
//
// 	VkImageCreateInfo createImage{};
// 	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
// 	createImage.imageType = VK_IMAGE_TYPE_2D;
// 	createImage.extent.width = width;
// 	createImage.extent.height = height;
// 	createImage.extent.depth = 1;
// 	createImage.mipLevels =	1;
// 	createImage.arrayLayers = 1;
// 	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
// 	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 	createImage.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
// 	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
// 	createImage.format = VK_FORMAT_R8G8B8A8_UNORM;
//
// 	//Creating image
// 	VmaAllocationCreateInfo vmaImageAllocationInfo{};
// 	vmaImageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
// 	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &createImage, &vmaImageAllocationInfo, &image, &allocation, nullptr) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create image!" << std::endl;
// 		return 1;
// 	}
//
//
// 	std::string imageAllocationName = "Texture: " + path;
// 	vmaSetAllocationName(SmoothieCore::getVulkanMemoryAllocator(), allocation, imageAllocationName.c_str());
//
//
// 	VkBuffer stagingBuffer = nullptr;
// 	VmaAllocation stagingBufferAllocation = nullptr;
//
// 	VmaAllocationCreateInfo stagingBufferAllocInfo = {};
// 	stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
// 	stagingBufferAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
// 	stagingBufferAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
//
// 	VkBufferCreateInfo stagingBufferCreateInfo{};
// 	stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
// 	stagingBufferCreateInfo.size = width * height * 4;
// 	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//
// 	if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &stagingBufferCreateInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingBufferAllocation, nullptr) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create staging buffer" << std::endl;
// 		return 1;
// 	}
//
// 	if (vmaCopyMemoryToAllocation(SmoothieCore::getVulkanMemoryAllocator(), data, stagingBufferAllocation, 0, width * height * 4) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to copy image data to staging buffer!" << std::endl;
// 		return 1;
// 	}
//
// 	//Free data
// 	if (data != nullptr)
// 	{
// 		stbi_image_free(data);
// 	}
//
//
// 	// auto commandBuffer = beginSingleTimeCommands();
// 	//
// 	// VkImageMemoryBarrier __barrier{};
// 	// __barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
// 	// __barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 	// __barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
// 	// __barrier.image = image;
// 	// __barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 	// __barrier.subresourceRange.baseMipLevel = 0;
// 	// __barrier.subresourceRange.levelCount = 1;
// 	// __barrier.subresourceRange.baseArrayLayer = 0;
// 	// __barrier.subresourceRange.layerCount = 1;
// 	// __barrier.srcAccessMask = 0;
// 	// __barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 	// vkCmdPipelineBarrier(commandBuffer.buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &__barrier);
// 	//
// 	//
// 	// VkBufferImageCopy region{};
// 	// region.bufferOffset = 0;
// 	// region.bufferRowLength = 0;
// 	// region.bufferImageHeight = 0;
// 	// region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 	// region.imageSubresource.mipLevel = 0;
// 	// region.imageSubresource.baseArrayLayer = 0;
// 	// region.imageSubresource.layerCount = 1;
// 	// region.imageOffset = { 0, 0, 0 };
// 	// region.imageExtent = { static_cast<unsigned int>(width), static_cast<unsigned int>(height), 1 };
// 	// vkCmdCopyBufferToImage(commandBuffer.buffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
// 	//
// 	// __barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
// 	// __barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 	// __barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 	// __barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
// 	// vkCmdPipelineBarrier(commandBuffer.buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &__barrier);
// 	//
// 	// endSingleTimeCommands(commandBuffer);
//
//
// 	vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), stagingBuffer, stagingBufferAllocation);
//
// 	VkImageViewCreateInfo imageViewInfo{};
// 	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
// 	imageViewInfo.image = image;
// 	imageViewInfo.format = createImage.format;
// 	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
// 	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 	imageViewInfo.subresourceRange.baseMipLevel = 0;
// 	imageViewInfo.subresourceRange.levelCount = 1;
// 	imageViewInfo.subresourceRange.baseArrayLayer = 0;
// 	imageViewInfo.subresourceRange.layerCount = 1;
// 	if (vkCreateImageView(SmoothieCore::getDevice(), &imageViewInfo, nullptr, &imageView) != VK_SUCCESS)
// 	{
// 		//std::cout << "Failed to create image view" << std::endl;
// 		return 1;
// 	}
//
// 	return 0;
// }
//
// int SmoothieEditor::Icon::create()
// {
//     if (load_icon(path, image, imageView, allocation) != 0)
//     {
//        // std::cout << "Failed to load image: " << path << std::endl;
//         return 1;
//     }
//
//     TextureID = ImGui_ImplVulkan_AddTexture(
//         sampler,
// 		imageView,
//         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
//     );
//
//     return 0;
// }
//
// void SmoothieEditor::Icon::destroy()
// {
//     ImGui_ImplVulkan_RemoveTexture(TextureID);
//     TextureID = nullptr;
// 	vkDestroyImageView(SmoothieCore::getDevice(), imageView, nullptr);
// 	imageView = nullptr;
// 	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), image, allocation);
// 	image = nullptr, allocation = nullptr;
// }