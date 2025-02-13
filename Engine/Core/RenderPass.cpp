#include "RenderPass.h"
#include "Core/DeviceDependency.h"
#define _SMOOTHIE_ENGINE
#include "Core/SmoothieCore.h"

VkRenderPass gBufferPass::renderPass = nullptr;
void gBufferPass::create()
{
	//Position attachment
	VkAttachmentDescription gPositionDescription{};
	gPositionDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	gPositionDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	gPositionDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	gPositionDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	gPositionDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	gPositionDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	gPositionDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	gPositionDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//Normals
	VkAttachmentDescription gNormalDescription{};
	gNormalDescription = gPositionDescription;
	gNormalDescription.format = VK_FORMAT_R16G16B16A16_SFLOAT;

	//Albedo channel
	VkAttachmentDescription gAlbedoDescription{};
	gAlbedoDescription = gPositionDescription;
	gAlbedoDescription.format = VK_FORMAT_R8G8B8A8_UNORM;

	//Metalic, roughness, AO
	VkAttachmentDescription gMRAODescription{};
	gMRAODescription = gPositionDescription;
	gMRAODescription.format = VK_FORMAT_R8G8B8A8_UNORM;

	//Depth 
	const auto depthFormat = DeviceDependencies::getSupportedDepthFormat();
	VkAttachmentDescription gDepthDescription{};
	gDepthDescription.format = depthFormat;
	gDepthDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	gDepthDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	gDepthDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	gDepthDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	gDepthDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	gDepthDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	gDepthDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


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


	VkSubpassDependency dependency{};
	dependency.srcSubpass = 0;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = 0;

	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = &dependency;


	vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &renderPass);

}

void gBufferPass::destroy()
{
	vkDestroyRenderPass(SmoothieCore::getDevice(), renderPass, nullptr);
	renderPass = nullptr;
}

VkRenderPass HDRPass::renderPass = nullptr;
void HDRPass::create()
{
	VkAttachmentDescription HDRDescription{};
	HDRDescription.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	HDRDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	HDRDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	HDRDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	HDRDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	HDRDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	HDRDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	HDRDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	const auto depthFormat = DeviceDependencies::getSupportedDepthFormat();
	VkAttachmentDescription depthDescription{};
	depthDescription.format = depthFormat;
	depthDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	depthDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	depthDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkAttachmentDescription attachments[2] =
	{
		HDRDescription, depthDescription
	};

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = nullptr;
	vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &renderPass);
}

void HDRPass::destroy()
{
	vkDestroyRenderPass(SmoothieCore::getDevice(), renderPass, nullptr);
	renderPass = nullptr;
}

VkRenderPass PBRPass::renderPass = nullptr;
void PBRPass::create()
{
	VkAttachmentDescription PBRDescription{};
	PBRDescription.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	PBRDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	PBRDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	PBRDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	PBRDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	PBRDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	PBRDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	PBRDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = nullptr;

	VkAttachmentDescription attachments[1] =
	{
		PBRDescription
	};

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	renderPassInfo.dependencyCount = 0;

	renderPassInfo.pDependencies = nullptr;
	vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &renderPass);
}

void PBRPass::destroy()
{
	vkDestroyRenderPass(SmoothieCore::getDevice(), renderPass, nullptr);
	renderPass = nullptr;
}
VkRenderPass BloomPass::renderPass;
void BloomPass::create()
{
	VkAttachmentDescription description{};
	description.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	description.samples = VK_SAMPLE_COUNT_1_BIT;
	description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = nullptr;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = dependency.dstSubpass = 0;
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &description;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = &dependency;

	vkCreateRenderPass(SmoothieCore::getDevice(), &renderPassInfo, nullptr, &renderPass);
}

void BloomPass::destroy()
{
	vkDestroyRenderPass(SmoothieCore::getDevice(), renderPass, nullptr);
	renderPass = nullptr;
}
