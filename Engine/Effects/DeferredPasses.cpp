#include "DeferredPasses.h"
#include "Core/SmoothieCore.h"
#include "Core/DeviceDependency.h"
#include "Effects/Deferred_Pipeline.h"
#include <iostream>

int Smoothie::DeferredRendering::Attachment_Color::create()
{
	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	createImage.extent = { SmoothieCore::getScrWidth() , SmoothieCore::getScrHeight() , 1 };
	createImage.mipLevels = 1;
	createImage.arrayLayers = 1;
	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createImage.format = imageFormat;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.priority = 1.0f;

	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &createImage, &allocInfo, &image, &imageAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create image!" << std::endl;
		return 1;
	}

	VkImageViewCreateInfo createImageView{};
	createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createImageView.image = image;
	createImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createImageView.format = imageFormat;
	createImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createImageView.subresourceRange.levelCount = 1;
	createImageView.subresourceRange.layerCount = 1;
	if (vkCreateImageView(SmoothieCore::getDevice(), &createImageView, nullptr, &imageView) != VK_SUCCESS)
	{
		std::cout << "Failed to create image view!" << std::endl;
		return 1;
	}

	return 0;
}

int Smoothie::DeferredRendering::Attachment_Depth::create()
{
	VkImageCreateInfo createImage{};
	createImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createImage.imageType = VK_IMAGE_TYPE_2D;
	createImage.extent = { SmoothieCore::getScrWidth() , SmoothieCore::getScrHeight() , 1 };
	createImage.mipLevels = 1;
	createImage.arrayLayers = 1;
	createImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	createImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createImage.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	createImage.samples = VK_SAMPLE_COUNT_1_BIT;
	createImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createImage.format = imageFormat;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.priority = 1.0f;
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &createImage, &allocInfo, &image, &imageAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create image" << std::endl;
		return 1;
	}

	VkImageViewCreateInfo createImageView{};
	createImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createImageView.image = image;
	createImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createImageView.format = imageFormat;
	createImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	createImageView.subresourceRange.baseMipLevel = 0;
	createImageView.subresourceRange.levelCount = 1;
	createImageView.subresourceRange.baseArrayLayer = 0;
	createImageView.subresourceRange.layerCount = 1;
	if (vkCreateImageView(SmoothieCore::getDevice(), &createImageView, nullptr, &imageView) != VK_SUCCESS)
	{
		std::cout << "Failed to create image view!" << std::endl;
		return 1;
	}

	return 0;
}

static inline void fill_color_gbuffer_attachment(VkAttachmentDescription& attachment, VkFormat format)
{
	attachment.format = format;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

int Smoothie::DeferredRendering::gBufferPass::create()
{

	gPosition.imageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
	if (gPosition.create() != 0)
	{
		std::cout << "Failed to create gPosition image!" << std::endl;
		return 1;
	}

	gNormal.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	if (gNormal.create() != 0)
	{
		std::cout << "Failed to create gNormal image!" << std::endl;
		return 1;
	}

	gAlbedo.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	if (gAlbedo.create() != 0)
	{
		std::cout << "Failed to create gAlbedo image!" << std::endl;
		return 1;
	}

	gDepth.imageFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
	if (gDepth.create() != 0)
	{
		std::cout << "Failed to create gDepth image!" << std::endl;
		return 1;
	}

	gMRAO.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	if (gMRAO.create() != 0)
	{
		std::cout << "Failed to create gMRAO image! " << std::endl;
		return 1;
	}


	VkAttachmentDescription gPositionDescription{};
	fill_color_gbuffer_attachment(gPositionDescription, VK_FORMAT_R32G32B32A32_SFLOAT);

	VkAttachmentDescription gNormalDescription{};
	fill_color_gbuffer_attachment(gNormalDescription, VK_FORMAT_R16G16B16A16_SFLOAT);

	VkAttachmentDescription gAlbedoDescription{};
	fill_color_gbuffer_attachment(gAlbedoDescription, VK_FORMAT_R8G8B8A8_UNORM);

	VkAttachmentDescription gMRAODescription{};
	fill_color_gbuffer_attachment(gMRAODescription, VK_FORMAT_R8G8B8A8_UNORM);

	const auto depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
	VkAttachmentDescription gDepthDescription{};
	gDepthDescription.format = depthFormat;
	gDepthDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	gDepthDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	gDepthDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	gDepthDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	gDepthDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	gDepthDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	gDepthDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


	VkAttachmentReference colorAttachmentRefs[4] =
	{
		{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, // Position
		{1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, // Normals
		{2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, // Albedo
		{3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }  // metalic, roughness, ao
	};

	VkAttachmentReference depthAttachmentRef =
	{
		4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};


	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 4;
	subpass.pColorAttachments = colorAttachmentRefs;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkAttachmentDescription attachments[5] =
	{
		gPositionDescription,
		gNormalDescription,
		gAlbedoDescription,
		gMRAODescription,
		gDepthDescription
	};

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 5;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = nullptr;
	if (vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS)
	{
		std::cout << "Failed to create gBuffer render pass!" << std::endl;
		return 1;
	}

	VkImageView attachmentsImageViews[] =
	{
		gPosition.getImageView(),
		gNormal.getImageView(),
		gAlbedo.getImageView(),
		gMRAO.getImageView(),
		gDepth.getImageView()
	};

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = render_pass;
	createInfo.attachmentCount = 5;
	createInfo.pAttachments = attachmentsImageViews;
	createInfo.width = SmoothieCore::getScrWidth();
	createInfo.height = SmoothieCore::getScrHeight();
	createInfo.layers = 1;
	if (vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &framebuffer) != VK_SUCCESS)
	{
		std::cout << "Failed to create gBuffer framebuffer!" << std::endl;
		return 1;
	}



	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayouts(createInfo.attachmentCount);
	for (int i = 0; i < createInfo.attachmentCount; i++)
	{
		VkDescriptorSetLayoutBinding& layoutBinding = descriptorSetLayouts[i];
		layoutBinding.binding = i;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBinding.pImmutableSamplers = nullptr;
	}
	//Descriptor set layout
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<unsigned int>(descriptorSetLayouts.size());
	layoutInfo.pBindings = descriptorSetLayouts.data();
	if (vkCreateDescriptorSetLayout(SmoothieCore::getDevice(), &layoutInfo, nullptr, &imagesDescriptorSetLayout) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor set layout" << std::endl;
		return 1;
	}

	VkDescriptorPoolSize poolSize{};
	poolSize.descriptorCount = createInfo.attachmentCount;
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;
	if (vkCreateDescriptorPool(SmoothieCore::getDevice(), &poolInfo, nullptr, &imagesDescriptorPool) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor pool!" << std::endl;
		return 1;
	}

	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = imagesDescriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &imagesDescriptorSetLayout;
	vkAllocateDescriptorSets(SmoothieCore::getDevice(), &descriptorSetAllocInfo, &imagesDescriptorSet);


	for (int i = 0; i < createInfo.attachmentCount; i++)
	{
		VkDescriptorImageInfo imgInfo{};
		imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgInfo.imageView = attachmentsImageViews[i];
		imgInfo.sampler = sampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = imagesDescriptorSet;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.dstBinding = i;
		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imgInfo;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
	}


	return 0;
}

int Smoothie::DeferredRendering::gBufferPass::resize_callback()
{

	//Recreate images
	gPosition.destroy();
	if (gPosition.create() != 0)
	{
		std::cout << "Failed to create gPosition image!" << std::endl;
		return 1;
	}

	gNormal.destroy();
	if (gNormal.create() != 0)
	{
		std::cout << "Failed to create gNormal image!" << std::endl;
		return 1;
	}

	gAlbedo.destroy();
	if (gAlbedo.create() != 0)
	{
		std::cout << "Failed to create gAlbedo image!" << std::endl;
		return 1;
	}

	gDepth.destroy();
	if (gDepth.create() != 0)
	{
		std::cout << "Failed to create gDepth image!" << std::endl;
		return 1;
	}

	gMRAO.destroy();
	if (gMRAO.create() != 0)
	{
		std::cout << "Failed to create gMRAO image! " << std::endl;
		return 1;
	}


	//Recreate framebuffer
	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr), framebuffer = nullptr;
	VkImageView attachmentsImageViews[] =
	{
		gPosition.getImageView(),
		gNormal.getImageView(),
		gAlbedo.getImageView(),
		gMRAO.getImageView(),
		gDepth.getImageView()
	};
	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = render_pass;
	createInfo.attachmentCount = 5;
	createInfo.pAttachments = attachmentsImageViews;
	createInfo.width = SmoothieCore::getScrWidth();
	createInfo.height = SmoothieCore::getScrHeight();
	createInfo.layers = 1;
	if (vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &framebuffer) != VK_SUCCESS)
	{
		std::cout << "Failed to create gBuffer framebuffer!" << std::endl;
		return 1;
	}

	//Update descriptors with new data
	for (int i = 0; i < createInfo.attachmentCount; i++)
	{
		VkDescriptorImageInfo imgInfo{};
		imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgInfo.imageView = attachmentsImageViews[i];
		imgInfo.sampler = sampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = imagesDescriptorSet;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.dstBinding = i;
		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imgInfo;
		vkUpdateDescriptorSets(SmoothieCore::getDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	return 0;
}

void Smoothie::DeferredRendering::gBufferPass::destroy()
{
	vkDestroyDescriptorPool(SmoothieCore::getDevice(), imagesDescriptorPool, nullptr), imagesDescriptorPool = nullptr, imagesDescriptorSet = nullptr;
	vkDestroyDescriptorSetLayout(SmoothieCore::getDevice(), imagesDescriptorSetLayout, nullptr), imagesDescriptorSetLayout = nullptr;
	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr), framebuffer = nullptr;
	vkDestroyRenderPass(SmoothieCore::getDevice(), render_pass, nullptr), render_pass = nullptr;
	gMRAO.destroy();
	gDepth.destroy();
	gAlbedo.destroy();
	gNormal.destroy();
	gPosition.destroy();
}

void Smoothie::DeferredRendering::gBufferPass::bindPass(VkCommandBuffer commandBuffer, unsigned int ImageIndex) const
{
	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.framebuffer = framebuffer;
	beginInfo.renderPass = render_pass;

	VkClearValue clearColor{};
	clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };

	VkClearValue depth{};
	depth.depthStencil.depth = 1.0f;

	static VkClearValue clearValues[] = { clearColor , clearColor , clearColor , clearColor , depth };
	beginInfo.clearValueCount = 5;
	beginInfo.pClearValues = clearValues;

	beginInfo.renderArea.extent = { SmoothieCore::getScrWidth(), SmoothieCore::getScrHeight() };
	beginInfo.renderArea.offset = { 0, 0 };
	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void Smoothie::DeferredRendering::gBufferPass::unbindPass(VkCommandBuffer commandBuffer, unsigned int ImageIndex) const
{
	vkCmdEndRenderPass(commandBuffer);

	VkMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		1, &memoryBarrier,
		0, nullptr,
		0, nullptr);
}

int Smoothie::DeferredRendering::HDRPass::create()
{
	HDR.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	if (HDR.create() != 0)
	{
		std::cout << "Failed to create HDR attachment image!" << std::endl;
		return 1;
	}

	VkAttachmentDescription HDRDescription{};
	fill_color_gbuffer_attachment(HDRDescription, VK_FORMAT_R16G16B16A16_SFLOAT);

	VkAttachmentDescription gDepthDescription{};
	gDepthDescription.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
	gDepthDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	gDepthDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	gDepthDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	gDepthDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	gDepthDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	gDepthDescription.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	gDepthDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


	VkAttachmentReference colorAttachmentRefs[1] =
	{
		{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, // HDR
	};

	VkAttachmentReference depthAttachmentRef =
	{
		1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};


	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = colorAttachmentRefs;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkAttachmentDescription attachments[2] =
	{
		HDRDescription,
		gDepthDescription
	};

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = nullptr;
	if (vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS)
	{
		std::cout << "Failed to create HDR render pass!" << std::endl;
		return 1;
	}


	VkImageView attachmentsImageViews[] =
	{
		HDR.getImageView(), gDepth.getImageView()
	};

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = render_pass;
	createInfo.attachmentCount = 2;
	createInfo.pAttachments = attachmentsImageViews;
	createInfo.width = SmoothieCore::getScrWidth();
	createInfo.height = SmoothieCore::getScrHeight();
	createInfo.layers = 1;

	if (vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &framebuffer) != VK_SUCCESS)
	{
		std::cout << "Failed to create HDR Framebuffer!" << std::endl;
		return 1;
	}

	return 0;
}

int Smoothie::DeferredRendering::HDRPass::resize_callback()
{
	HDR.destroy();
	if (HDR.create() != 0)
	{
		std::cout << "Failed to create HDR attachment image!" << std::endl;
		return 1;
	}

	VkImageView attachmentsImageViews[] =
	{
		HDR.getImageView(), gDepth.getImageView()
	};

	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr), framebuffer = nullptr;
	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = render_pass;
	createInfo.attachmentCount = 2;
	createInfo.pAttachments = attachmentsImageViews;
	createInfo.width = SmoothieCore::getScrWidth();
	createInfo.height = SmoothieCore::getScrHeight();
	createInfo.layers = 1;
	if (vkCreateFramebuffer(SmoothieCore::getDevice(), &createInfo, nullptr, &framebuffer) != VK_SUCCESS)
	{
		std::cout << "Failed to create HDR Framebuffer!" << std::endl;
		return 1;
	}

	return 0;
}

void Smoothie::DeferredRendering::HDRPass::destroy()
{
	vkDestroyFramebuffer(SmoothieCore::getDevice(), framebuffer, nullptr);
	framebuffer = nullptr;

	vkDestroyRenderPass(SmoothieCore::getDevice(), render_pass, nullptr);
	render_pass = nullptr;

	HDR.destroy();

}

void Smoothie::DeferredRendering::HDRPass::bindPass(VkCommandBuffer commandBuffer, unsigned int ImageIndex) const
{
	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.framebuffer = framebuffer;
	beginInfo.renderPass = render_pass;
	VkClearValue clearColor{};
	clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	VkClearValue depth{};
	depth.depthStencil.depth = 1.0f;
	static VkClearValue clearValues[] = { clearColor, depth };
	beginInfo.clearValueCount = 2;
	beginInfo.pClearValues = clearValues;
	beginInfo.renderArea.extent = { SmoothieCore::getScrWidth(), SmoothieCore::getScrHeight() };
	beginInfo.renderArea.offset = { 0, 0 };
	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void Smoothie::DeferredRendering::HDRPass::unbindPass(VkCommandBuffer commandBuffer, unsigned int ImageIndex) const
{
	vkCmdEndRenderPass(commandBuffer);

	VkMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		1, &memoryBarrier,
		0, nullptr,
		0, nullptr);
}
