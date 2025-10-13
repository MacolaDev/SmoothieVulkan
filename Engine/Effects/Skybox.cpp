#include "Skybox.h"
#include "Core/SmoothieCore.h"
#include "Core/Multithreading.h"

#include "Effects/Deferred_Core.h"

#include "stb_image.h"

#include <iostream>
#include <array>

int Smoothie::DeferredRendering::SkyboxCubemapTexture::create()
{
	if (Texture2DModelSampler == nullptr)
	{
		std::cout << "Texture2DModelSampler is not a valid sampler object!" << std::endl;
		return 1;
	}

	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	createImage.extent.width = 512;
	createImage.extent.height = 512;
	createImage.extent.depth = 1;
	createImage.mipLevels = 1;
	createImage.arrayLayers = 6;
	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createImage.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	createImage.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.priority = 1.0f;
	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &createImage, &allocInfo, &image, &allocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create Image!" << std::endl;
		return 1;
	}

	VkImageViewCreateInfo createImageView{};
	createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createImageView.image = image;
	createImageView.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	createImageView.format = createImage.format;
	createImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createImageView.subresourceRange.baseMipLevel = 0;
	createImageView.subresourceRange.levelCount = 1;
	createImageView.subresourceRange.baseArrayLayer = 0;
	createImageView.subresourceRange.layerCount = 6;
	if (vkCreateImageView(SmoothieCore::getDevice(), &createImageView, nullptr, &imageView) != VK_SUCCESS)
	{
		std::cout << "Failed to create image view!" << std::endl;
		return 1;
	}
	
	//********************* Fill up the image with nice values ****************************//
	auto commandBuffer = beginSingleTimeCommands();
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = createImage.initialLayout;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	barrier.image = image;
	barrier.subresourceRange = createImageView.subresourceRange;
	vkCmdPipelineBarrier(commandBuffer.buffer,
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

	vkCmdClearColorImage(commandBuffer.buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &_clearColorValue, 1, &createImageView.subresourceRange);
	
	barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	vkCmdPipelineBarrier(commandBuffer.buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);
	endSingleTimeCommands(commandBuffer);



	//**************************************** Descriptors ****************************************//
	VkDescriptorSetLayoutBinding _MapBindings{};
	_MapBindings.binding = 0;
	_MapBindings.descriptorCount = 1;
	_MapBindings.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	_MapBindings.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo _descriptorSetLayoutCreateInfo{};
	_descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	_descriptorSetLayoutCreateInfo.bindingCount = 1;
	_descriptorSetLayoutCreateInfo.pBindings = &_MapBindings;
	if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &_descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor set layout!" << std::endl;
		return 1;
	}

	VkDescriptorPoolCreateInfo _poolCreateInfo{};
	_poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	_poolCreateInfo.maxSets = 1;
	VkDescriptorPoolSize _poolSize = {};
	_poolSize.descriptorCount = 1;
	_poolSize.type = _MapBindings.descriptorType;
	_poolCreateInfo.poolSizeCount = 1;
	_poolCreateInfo.pPoolSizes = &_poolSize;
	if (vkCreateDescriptorPool(SmoothieCore::getDevice(), &_poolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor pool!" << std::endl;
		return 1;
	}

	VkDescriptorSetAllocateInfo _allocInfo = {};
	_allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	_allocInfo.descriptorPool = descriptorPool;
	_allocInfo.descriptorSetCount = 1;
	_allocInfo.pSetLayouts = &descriptorSetLayout;
	vkAllocateDescriptorSets(SmoothieCore::getDevice(), &_allocInfo, &descriptorSet);


	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = imageView;
	imageInfo.sampler = Texture2DModelSampler;

	VkWriteDescriptorSet _WriteIrradinaceSet{};
	_WriteIrradinaceSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	_WriteIrradinaceSet.dstSet = descriptorSet;
	_WriteIrradinaceSet.dstArrayElement = 0;
	_WriteIrradinaceSet.descriptorType = _MapBindings.descriptorType;
	_WriteIrradinaceSet.descriptorCount = 1;
	_WriteIrradinaceSet.pBufferInfo = nullptr;
	_WriteIrradinaceSet.pImageInfo = &imageInfo;
	_WriteIrradinaceSet.dstBinding = 0;
	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &_WriteIrradinaceSet, 0, nullptr);


	return 0;
}

void generateViewMatrices(std::array<SmoothieMath::Matrix4x4, 6>& matrices)
{
	SmoothieMath::Matrix4x4 matrix;

	matrix.lookAtMatrix({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
	matrices[0] = matrix;
	transpose(matrices[0]);

	matrix.lookAtMatrix({ 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
	matrices[1] = matrix;
	transpose(matrices[1]);

	matrix.lookAtMatrix({ 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });
	matrices[3] = matrix;
	transpose(matrices[3]);

	matrix.lookAtMatrix({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
	matrices[2] = matrix;
	transpose(matrices[2]);

	matrix.lookAtMatrix({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f });
	matrices[4] = matrix;
	transpose(matrices[4]);

	matrix.lookAtMatrix({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f });
	matrices[5] = matrix;
	transpose(matrices[5]);
}

int Smoothie::DeferredRendering::SkyboxCubemapTexture::create_from_hdri_image(const std::string& filepath)
{
	if ((image == nullptr) || (allocation == nullptr) || (imageView == nullptr))
	{
		if (create() != 0)
		{
			std::cout << "Failed to create base image!" << std::endl;
			return 1;
		}
	}

	//******************************** Load data from file **********************************//
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float* data = stbi_loadf(filepath.c_str(), &width, &height, &nrComponents, 4);
	if (data == nullptr)
	{
		std::cout << "Cant load HDR image: " + filepath << std::endl;
		return 1;
	}



	//******************************** Creating base image and image view **********************************//
	VkImage _loadedImage = nullptr;
	VkImageView _loadedImageView = nullptr;
	VmaAllocation _loadedImageAllocation = nullptr;

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
	createImage.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	VmaAllocationCreateInfo vmaImageAllocationInfo{};
	vmaImageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &createImage, &vmaImageAllocationInfo, &_loadedImage, &_loadedImageAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create image!" << std::endl;
		return 1;
	}

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = _loadedImage;
	imageViewInfo.format = createImage.format;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	if (vkCreateImageView(SmoothieCore::getDevice(), &imageViewInfo, nullptr, &_loadedImageView) != VK_SUCCESS)
	{
		std::cout << "Failed to create Image view for loaded image!" << std::endl;
		return 1;
	}


	//******************************** Creating staging buffer **********************************//
	VkBuffer stagingBuffer = nullptr;
	VmaAllocation stagingBufferAllocation = nullptr;
	VmaAllocationCreateInfo stagingBufferAllocInfo = {};
	stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	stagingBufferAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	stagingBufferAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	
	VkMemoryRequirements __memoryRequirements = {};
	vkGetImageMemoryRequirements(SmoothieCore::getDevice(), _loadedImage, &__memoryRequirements);
	
	VkBufferCreateInfo stagingBufferCreateInfo{};
	stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferCreateInfo.size = __memoryRequirements.size;
	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(), &stagingBufferCreateInfo, &stagingBufferAllocInfo, &stagingBuffer, &stagingBufferAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create staging buffer!" << std::endl;
		return 1;
	}

	if (vmaCopyMemoryToAllocation(SmoothieCore::getVulkanMemoryAllocator(), data, stagingBufferAllocation, 0, stagingBufferCreateInfo.size) != VK_SUCCESS)
	{
		std::cout << "Failed to copy memoty to allocation!" << std::endl;
		return 1;
	}
	if (data != nullptr)
	{
		stbi_image_free(data);
		data = nullptr;
	}

	
	//******************************** Transfer data to the GPU **********************************//
	auto& commands = beginSingleTimeCommands();
	VkImageMemoryBarrier _barrier = {};
	_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	_barrier.pNext = nullptr;
	_barrier.srcAccessMask = 0;
	_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	_barrier.image = _loadedImage;
	_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	_barrier.subresourceRange.baseMipLevel = 0;
	_barrier.subresourceRange.levelCount = 1;
	_barrier.subresourceRange.baseArrayLayer = 0;
	_barrier.subresourceRange.layerCount = 1;
	vkCmdPipelineBarrier(commands.buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &_barrier);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { static_cast<unsigned int>(width), static_cast<unsigned int>(height), 1};
	vkCmdCopyBufferToImage(commands.buffer, stagingBuffer, _loadedImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	vkCmdPipelineBarrier(commands.buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &_barrier);
	endSingleTimeCommands(commands);



	//******************************** Destroy staging buffer **********************************//
	vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), stagingBuffer, stagingBufferAllocation);
	stagingBuffer = nullptr, stagingBufferAllocation = nullptr;


	//******************************** Render pass **********************************//
	VkRenderPass rgb32f_RenderPass = nullptr;
	VkAttachmentDescription attachmentDescription{};
	attachmentDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	VkAttachmentReference attachmentRefrence{};
	attachmentRefrence.attachment = 0;
	attachmentRefrence.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &attachmentRefrence;
	subpass.pDepthStencilAttachment = nullptr;
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &attachmentDescription;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	if (vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &rgb32f_RenderPass) != VK_SUCCESS)
	{
		std::cout << "Failed to create render pass!" << std::endl;
		return 1;
	}


	//******************************** Render Target and framebuffer **********************************//
	VkImage targetImage = nullptr;
	VkImageView targetImageView = nullptr;
	VmaAllocation targetImageAllocation = nullptr;

	VkImageCreateInfo _targeImagetCreateImage{};
	_targeImagetCreateImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	_targeImagetCreateImage.imageType = VK_IMAGE_TYPE_2D;
	_targeImagetCreateImage.extent = { 512 , 512 , 1 };
	_targeImagetCreateImage.mipLevels = 1;
	_targeImagetCreateImage.arrayLayers = 1;
	_targeImagetCreateImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	_targeImagetCreateImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_targeImagetCreateImage.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	_targeImagetCreateImage.samples = VK_SAMPLE_COUNT_1_BIT;
	_targeImagetCreateImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	_targeImagetCreateImage.format = attachmentDescription.format;
	VmaAllocationCreateInfo _targeImagetAllocInfo = {};
	_targeImagetAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	_targeImagetAllocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	_targeImagetAllocInfo.priority = 1.0f;
	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_targeImagetCreateImage, &_targeImagetAllocInfo, &targetImage, &targetImageAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create target image for rendering!" << std::endl;
		return 1;
	}

	VkImageViewCreateInfo createImageView{};
	createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createImageView.image = targetImage;
	createImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createImageView.format = attachmentDescription.format;
	createImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createImageView.subresourceRange.levelCount = 1;
	createImageView.subresourceRange.layerCount = 1;
	if (vkCreateImageView(SmoothieCore::getDevice(), &createImageView, nullptr, &targetImageView) != VK_SUCCESS)
	{
		std::cout << "Failed to create target image view for rendering!" << std::endl;
		return 1;
	}

	VkFramebuffer framebuffer = nullptr;
	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = rgb32f_RenderPass;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &targetImageView;
	createInfo.width = _targeImagetCreateImage.extent.width;
	createInfo.height = _targeImagetCreateImage.extent.height;
	createInfo.layers = 1;
	if (vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &framebuffer) != VK_SUCCESS)
	{
		std::cout << "Failed to create target framebuffer!" << std::endl;
		return 1;
	}


	//******************************** Descriptors **********************************//
	VkDescriptorPool descriptorPool = nullptr;
	VkDescriptorSet descriptorSet = nullptr;
	VkDescriptorSetLayout descriptorSetLayout = nullptr;

	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;
	if (vkCreateDescriptorPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor pool!" << std::endl;
		return 1;
	}


	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;
	if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor set layout!" << std::endl;
		return 1;
	}


	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
	vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &descriptorSet);

	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgInfo.imageView = _loadedImageView;
	imgInfo.sampler = Texture2DModelSampler;
	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pImageInfo = &imgInfo;
	vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);

	//******************************** Pipeline **********************************//
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;

	VkPushConstantRange range = {};
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	range.offset = 0;
	range.size = sizeof(SmoothieMath::Matrix4x4);
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &range;
	if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create pipeline layout!" << std::endl;
		return 1;
	}

	VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
	vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderPipelineCreateInfo.pName = "main";
	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderPipelineCreateInfo.module = pbsVertexShader;
	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderPipelineCreateInfo.pName = "main";
	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderPipelineCreateInfo.module = fragmentShader;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;
	DefaultPipelineState __state;
	__state.populate_pipeline(pipelineInfo);
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = rgb32f_RenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;
	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create graphics pipeline!" << std::endl;
		return 1;
	}
	std::array<SmoothieMath::Matrix4x4, 6> viewMatrices;
	generateViewMatrices(viewMatrices);


	//******************************** Draw **********************************//
	auto& commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier cubemapTransferDST{};
	cubemapTransferDST.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	cubemapTransferDST.srcAccessMask = 0;
	cubemapTransferDST.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	cubemapTransferDST.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	cubemapTransferDST.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	cubemapTransferDST.image = this->image;
	cubemapTransferDST.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	cubemapTransferDST.subresourceRange.baseMipLevel = 0;
	cubemapTransferDST.subresourceRange.levelCount = 1;
	cubemapTransferDST.subresourceRange.baseArrayLayer = 0;
	cubemapTransferDST.subresourceRange.layerCount = 6;
	vkCmdPipelineBarrier(commandBuffer.buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &cubemapTransferDST);

	for (size_t i = 0; i < 6; i++)
	{
		VkRenderPassBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		beginInfo.framebuffer = framebuffer;
		beginInfo.renderPass = rgb32f_RenderPass;
		VkClearValue clearColor{};
		clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		beginInfo.clearValueCount = 1;
		beginInfo.pClearValues = &clearColor;
		beginInfo.renderArea.extent.height = _targeImagetCreateImage.extent.height;
		beginInfo.renderArea.extent.width = _targeImagetCreateImage.extent.width;
		beginInfo.renderArea.offset = { 0, 0 };
		vkCmdBeginRenderPass(commandBuffer.buffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(_targeImagetCreateImage.extent.width);
		viewport.height = static_cast<float>(_targeImagetCreateImage.extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer.buffer, 0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent.height = _targeImagetCreateImage.extent.height;
		scissor.extent.width = _targeImagetCreateImage.extent.width;
		vkCmdSetScissor(commandBuffer.buffer, 0, 1, &scissor);
		vkCmdBindDescriptorSets(commandBuffer.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
		vkCmdPushConstants(commandBuffer.buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SmoothieMath::Matrix4x4), &viewMatrices[i]);
		vkCmdDraw(commandBuffer.buffer, 36, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffer.buffer);
		
		//Wait for all graphics to finish its work
		VkMemoryBarrier _memory_barrier = {};
		_memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		vkCmdPipelineBarrier(commandBuffer.buffer, 
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 
			1, &_memory_barrier,
			0, nullptr, 
			0, nullptr);


		//Copy the image
		VkImageCopy copyRegions{};
		copyRegions.extent.height = scissor.extent.height;
		copyRegions.extent.width = scissor.extent.width;
		copyRegions.extent.depth = 1;
		copyRegions.srcOffset = copyRegions.dstOffset = { 0, 0, 0 };
		copyRegions.srcSubresource.baseArrayLayer = 0;
		copyRegions.srcSubresource.layerCount = 1;
		copyRegions.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegions.dstSubresource.baseArrayLayer = i;
		copyRegions.dstSubresource.layerCount = 1;
		copyRegions.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		vkCmdCopyImage(commandBuffer.buffer,
			targetImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			this->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &copyRegions);


		//Wait for all copying to finish its work before drawing again
		_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkCmdPipelineBarrier(commandBuffer.buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			0,
			1, &_memory_barrier,
			0, nullptr,
			0, nullptr);

	}

	//Makes cubemap ready for sampling in shader
	VkImageMemoryBarrier imageMemoryBarier{};
	imageMemoryBarier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarier.image = this->image;
	imageMemoryBarier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarier.subresourceRange.levelCount = 1;
	imageMemoryBarier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarier.subresourceRange.layerCount = 6;
	vkCmdPipelineBarrier(commandBuffer.buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarier);

	endSingleTimeCommands(commandBuffer);
	

	//***************************** Cleanup ************************************//
	vkDestroyPipeline(SmoothieCore::getDevice(), pipeline, nullptr), pipeline = nullptr;
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), pipelineLayout, nullptr), pipelineLayout = nullptr;
	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), descriptorSetLayout, nullptr), descriptorSetLayout = nullptr;
	vkDestroyDescriptorPool(SmoothieCore::getDevice(), descriptorPool, nullptr), descriptorPool = nullptr, descriptorSet = nullptr;
	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr), framebuffer = nullptr;
	vkDestroyImageView(SmoothieCore::getDevice(), targetImageView, nullptr), targetImageView = nullptr;
	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), targetImage, targetImageAllocation), targetImage = nullptr, targetImageAllocation = nullptr;
	vkDestroyRenderPass(SmoothieCore::getDevice(), rgb32f_RenderPass, nullptr), rgb32f_RenderPass = nullptr;
	vkDestroyImageView(SmoothieCore::getDevice(), _loadedImageView, nullptr), _loadedImageView = nullptr;
	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), _loadedImage, _loadedImageAllocation);
	_loadedImage = nullptr, _loadedImageAllocation = nullptr;
	
	return 0;
}

void Smoothie::DeferredRendering::SkyboxCubemapTexture::destroy()
{

	vkDestroyDescriptorPool(SmoothieCore::getDevice(), descriptorPool, nullptr);
	descriptorPool = nullptr;

	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), descriptorSetLayout, nullptr);
	descriptorSetLayout = nullptr;

	vkDestroyImageView(SmoothieCore::getDevice(), imageView, nullptr);
	imageView = nullptr;
	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), image, allocation);
	image = nullptr, allocation = nullptr;
}

int Smoothie::DeferredRendering::Skybox::create()
{
	if (renderPass == nullptr)
	{
		std::cout << "Invalid render pass object!" << std::endl;
		return 1;
	}

	const VkDescriptorSetLayout descriptorSets[] =
	{
		SmoothieCore::getCameraDescriptorSetLayout(0),
		this->drawerClassDescriptorSetLayout,
		HDRCubemap_descriptorSetLayout
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = sizeof(descriptorSets) / sizeof(descriptorSets[0]);
	pipelineLayoutInfo.pSetLayouts = descriptorSets;
	if (vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != 0)
	{
		std::cout << "Failed to create pipeline layout!" << std::endl;
		return 1;
	}


	VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
	vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderPipelineCreateInfo.pName = "main";
	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderPipelineCreateInfo.module = skyboxVertex;

	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderPipelineCreateInfo.pName = "main";
	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderPipelineCreateInfo.module = skyboxFragment;

	const VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;
	Smoothie::DefaultPipelineState __state;
	__state.populate_pipeline(pipelineInfo);
	auto& depthState = __state.getDepthStencilStateCreateInfo();
	depthState.depthTestEnable = true;
	pipelineInfo.pDepthStencilState = &depthState;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.layout = pipelineLayout;
	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), nullptr, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create global illumination pipeline!" << std::endl;
		return 1;
	}

	return 0;
}

void Smoothie::DeferredRendering::Skybox::draw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, unsigned int ImageID) const
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	SmoothieCore::setViewport(commandBuffer);
	SmoothieCore::setScissor(commandBuffer);

	const VkDescriptorSet _descriptors[3] =
	{
		SmoothieCore::getCameraDescriptorSet(),
		descriptorSet,
		HDRCubemap_descriptorSet
	};

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 3, _descriptors, 0, 0);
	vkCmdDraw(commandBuffer, 36, 1, 0, 0);
}

void Smoothie::DeferredRendering::Skybox::destroy()
{
	vkDestroyPipeline(SmoothieCore::getDevice(), pipeline, nullptr), pipeline = nullptr;
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), pipelineLayout, nullptr), pipelineLayout = nullptr;
}
