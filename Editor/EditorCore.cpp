//
// Created by macola on 12/13/25.
//

#include "EditorCore.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <iostream>

int SmoothieEditor::Core::create()
{
	auto* draw_class = dynamic_cast<Smoothie::DeferredRendering::Drawing*>(SmoothieCore::getDrawingClass());
	if (draw_class == nullptr)
	{
		std::cout << "Draw class must be a child of DeferredRendering::Drawing class!" << std::endl;
		return 1;
	}

	m_drawing_class = std::dynamic_pointer_cast<Smoothie::DeferredRendering::Drawing>(SmoothieCore::getDrawingClassPtr());

	//*****************************************
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.ApiVersion = VK_API_VERSION_1_3;
	init_info.Instance = SmoothieCore::getInstance();
	init_info.PhysicalDevice = SmoothieCore::getPhysicalDevice();
	init_info.Device = SmoothieCore::getDevice();
	init_info.QueueFamily = SmoothieCore::getQueueFamilyGraphicsIndex();
	init_info.Queue = SmoothieCore::getGraphicsQueue();
	init_info.PipelineCache = nullptr;
	init_info.DescriptorPool = nullptr;
	init_info.MinImageCount = SmoothieCore::getSwapchainImages().size();
	init_info.ImageCount = SmoothieCore::getSwapchainImages().size();
	init_info.RenderPass = SmoothieCore::getDefaultRenderPass();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;
	init_info.DescriptorPoolSize = 8;
	ImGui_ImplVulkan_Init(&init_info);

    VkImageCreateInfo _resultImageCreateInfo{};
	_resultImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	_resultImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	_resultImageCreateInfo.extent.width = SmoothieCore::getScrWidth();
	_resultImageCreateInfo.extent.height = SmoothieCore::getScrHeight();
	_resultImageCreateInfo.extent.depth = 1;
	_resultImageCreateInfo.mipLevels = 1;
	_resultImageCreateInfo.arrayLayers = 1;
	_resultImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	_resultImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_resultImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	_resultImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	_resultImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	_resultImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	VmaAllocationCreateInfo vmaImageAllocationInfo{};
	vmaImageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_resultImageCreateInfo, &vmaImageAllocationInfo, &m_resultImage, &m_resultImageAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create result image!" << std::endl;
		return 1;
	}

	VkImageViewCreateInfo _resultImageViewCreateInfo{};
	_resultImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	_resultImageViewCreateInfo.image = m_resultImage;
	_resultImageViewCreateInfo.format = _resultImageCreateInfo.format;
	_resultImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	_resultImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	_resultImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	_resultImageViewCreateInfo.subresourceRange.levelCount = 1;
	_resultImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	_resultImageViewCreateInfo.subresourceRange.layerCount = 1;
	if (vkCreateImageView(SmoothieCore::getDevice(), &_resultImageViewCreateInfo, nullptr, &m_resultImageView) != VK_SUCCESS)
	{
		std::cout << "Failed to create Image view for result!" << std::endl;
		return 1;
	}
	m_resultImageTextureID = ImGui_ImplVulkan_AddTexture(draw_class->getSampler("Texture2DModelSampler"), m_resultImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


	//********************************************* Present pipeline *********************************************//
	VkPipelineShaderStageCreateInfo vertexShaderPipelineCreateInfo{};
	vertexShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderPipelineCreateInfo.pName = "main";
	vertexShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderPipelineCreateInfo.module = draw_class->getSystemShaderModule("SYSTEM");

	VkPipelineShaderStageCreateInfo fragmentShaderPipelineCreateInfo{};
	fragmentShaderPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderPipelineCreateInfo.pName = "main";
	fragmentShaderPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderPipelineCreateInfo.module = draw_class->getSystemShaderModule("PRESENT");

	const VkPipelineShaderStageCreateInfo stages[] = { vertexShaderPipelineCreateInfo, fragmentShaderPipelineCreateInfo };
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;
	Smoothie::DefaultPipelineState __state;
	__state.populate_pipeline(pipelineInfo);
	pipelineInfo.renderPass = nullptr;
	pipelineInfo.layout = draw_class->getPresentPipelineLayout();

	VkPipelineRenderingCreateInfo _pipeline_rendering_create_info = {};
	_pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	_pipeline_rendering_create_info.pNext = nullptr;
	_pipeline_rendering_create_info.colorAttachmentCount = 1;
	const VkFormat _format = {VK_FORMAT_R8G8B8A8_UNORM};
	_pipeline_rendering_create_info.pColorAttachmentFormats = &_format;
	_pipeline_rendering_create_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
	_pipeline_rendering_create_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
	pipelineInfo.pNext = &_pipeline_rendering_create_info;

	if (vkCreateGraphicsPipelines(SmoothieCore::getDevice(), nullptr, 1, &pipelineInfo, nullptr, &m_resultPipeline) != VK_SUCCESS)
	{
		std::cout << "Failed to create resulting pipeline!" << std::endl;
		return 1;
	}



    //********************************************* Shaders *********************************************//
    for (const auto& _shader: m_editor_shader_files)
    {
        // if (Smoothie::Add_system_shader(_shader, m_editor_shaders) != 0)
        // {
        //     std::cout << "Failed to create shader modules!" << std::endl;
        //     return 1;
        // }
    }


    //********************************************* Tools *********************************************//
    for (auto& [key, tool]: m_tools)
    {
        if (tool->create() != 0)
        {
			std::cout << "Failed to create a tool"  << std::endl;
            return 1;
        }
    }

    return 0;
}

void SmoothieEditor::Core::on_command_record_time(VkCommandBuffer commandBuffer, unsigned imageIndex)
{
	//****************************** GPU compute work ******************************//
	for (auto& [key, tool]: m_tools)
	{
		tool->on_command_recording_compute(commandBuffer);
	}

	//****************************** Final image draw  ******************************//
	VkImageMemoryBarrier __barrier = {};
	__barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	__barrier.pNext = nullptr;
	__barrier.image = m_resultImage;
	__barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	__barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	__barrier.srcAccessMask = 0;
	__barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	__barrier.subresourceRange.layerCount = 1;
	__barrier.subresourceRange.baseMipLevel = 0;
	__barrier.subresourceRange.levelCount = 1;
	__barrier.subresourceRange.baseMipLevel = 0;
	__barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkCmdPipelineBarrier(commandBuffer,  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &__barrier);

	VkRenderingInfo __renderingInfo = {};
	__renderingInfo.pNext = nullptr;
	__renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	__renderingInfo.colorAttachmentCount = 1;
	__renderingInfo.layerCount = 1;
	__renderingInfo.renderArea.offset = { 0, 0 };
	__renderingInfo.renderArea.extent = {SmoothieCore::getScrWidth(), SmoothieCore::getScrHeight() };

	VkRenderingAttachmentInfo __attachmentInfo = {};
	__attachmentInfo.pNext = nullptr;
	__attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	__attachmentInfo.imageView = m_resultImageView;
	__attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	__attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	__attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	__attachmentInfo.clearValue.color.float32[0] = 0.0f;
	__attachmentInfo.clearValue.color.float32[1] = 0.0f;
	__attachmentInfo.clearValue.color.float32[2] = 0.0f;
	__attachmentInfo.clearValue.color.float32[3] = 1.0f;
	__renderingInfo.pColorAttachments = &__attachmentInfo;
	vkCmdBeginRendering(commandBuffer, &__renderingInfo);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_resultPipeline);
	const VkDescriptorSet __descriptors[3] =
	{
		SmoothieCore::getCameraDescriptorSet(),
		m_drawing_class->getDescriptorSet(),
		m_drawing_class->getPresentDescriptorSet()
	};
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_drawing_class->getPresentPipelineLayout(), 0, 3, __descriptors, 0, nullptr);
	SmoothieCore::setScissor(commandBuffer);
	SmoothieCore::setViewport(commandBuffer);
	vkCmdDraw(commandBuffer, 6, 1, 0, 0);


	for (auto& [key, tool]: m_tools)
	{
		tool->on_command_recording_graphics(commandBuffer);
	}

	vkCmdEndRendering(commandBuffer);
	__barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	__barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	__barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	__barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &__barrier);



	//****************************** UI draw  ******************************//
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = { 0, 0 };

    auto _id = ImGui::GetID("DockingSpaceID");
    ImGui::DockSpaceOverViewport(_id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoDockingOverCentralNode | ImGuiDockNodeFlags_AutoHideTabBar, nullptr);


    //This thing disables tab bar from showing in the main note.
    //We dont need it anyway since no window can be docked to it anyway since its a central node.
    //This is from internal API for docking, I hope it wont change in the future :D
    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoTabBar;
    ImGui::SetNextWindowClass(&window_class);
    ImGui::SetNextWindowDockID(_id);
    ImGui::Begin("Canvas", nullptr, 0);
    ImGui::Image((ImTextureID)(m_resultImageTextureID), ImGui::GetContentRegionAvail());
    for (auto& [key, tool]: m_tools)
    {
        tool->on_ui_icon_draw();
    }
    ImGui::End();
	ImGui::ShowDemoWindow();
    for (auto& [key, tool]: m_tools)
    {
        tool->on_ui_window_draw();
    }
    ImGui::Render();

	//****************************** Draw everything to the screen *******************************//
	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	VkClearValue clearValue{};
	clearValue.depthStencil = { 0.0f, 0 };
	clearValue.color.float32[0] = 0.0f;
	clearValue.color.float32[1] = 0.0f;
	clearValue.color.float32[2] = 0.0f;
	clearValue.color.float32[3] = 1.0f;
	beginInfo.clearValueCount = 1;
	beginInfo.pClearValues = &clearValue;
	beginInfo.renderArea.offset = { 0, 0 };
	beginInfo.renderArea.extent.height = SmoothieCore::getScrHeight();
	beginInfo.renderArea.extent.width = SmoothieCore::getScrWidth();
	beginInfo.renderPass = SmoothieCore::getDefaultRenderPass();
	beginInfo.framebuffer = SmoothieCore::getSwapchainFramebuffer(imageIndex);
	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
	ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
    vkCmdEndRenderPass(commandBuffer);

}

void SmoothieEditor::Core::on_command_submit_time()
{
    for (auto& [key, tool]: m_tools)
    {
        tool->on_command_execution_finish();
    }
}

int SmoothieEditor::Core::resize_callback()
{

	ImGui_ImplVulkan_RemoveTexture(m_resultImageTextureID), m_resultImageTextureID = nullptr;

	VkImageCreateInfo _resultImageCreateInfo{};
	_resultImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	_resultImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	_resultImageCreateInfo.extent.width = SmoothieCore::getScrWidth();
	_resultImageCreateInfo.extent.height = SmoothieCore::getScrHeight();
	_resultImageCreateInfo.extent.depth = 1;
	_resultImageCreateInfo.mipLevels = 1;
	_resultImageCreateInfo.arrayLayers = 1;
	_resultImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	_resultImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_resultImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	_resultImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	_resultImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	_resultImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	VmaAllocationCreateInfo vmaImageAllocationInfo{};
	vmaImageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_resultImage, m_resultImageAllocation);
	m_resultImage = nullptr, m_resultImageAllocation = nullptr;

	if (vmaCreateImage(SmoothieCore::getVulkanMemoryAllocator(), &_resultImageCreateInfo, &vmaImageAllocationInfo, &m_resultImage, &m_resultImageAllocation, nullptr) != VK_SUCCESS)
	{
		std::cout << "Failed to create result image!" << std::endl;
		return 1;
	}

	VkImageViewCreateInfo _resultImageViewCreateInfo{};
	_resultImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	_resultImageViewCreateInfo.image = m_resultImage;
	_resultImageViewCreateInfo.format = _resultImageCreateInfo.format;
	_resultImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	_resultImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	_resultImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	_resultImageViewCreateInfo.subresourceRange.levelCount = 1;
	_resultImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	_resultImageViewCreateInfo.subresourceRange.layerCount = 1;
	vkDestroyImageView(SmoothieCore::getDevice(), m_resultImageView, nullptr), m_resultImageView = nullptr;
	if (vkCreateImageView(SmoothieCore::getDevice(), &_resultImageViewCreateInfo, nullptr, &m_resultImageView) != VK_SUCCESS)
	{
		std::cout << "Failed to create Image view for result!" << std::endl;
		return 1;
	}

	auto* draw_class = dynamic_cast<Smoothie::DeferredRendering::Drawing*>(SmoothieCore::getDrawingClass());
	if (draw_class == nullptr)
	{
		std::cout << "Draw class must be a child of DeferredRendering::Drawing class!" << std::endl;
		return 1;
	}

	const VkSampler _sampler = draw_class->getSampler("Texture2DModelSampler");
	assert(_sampler != nullptr);
	if (_sampler == nullptr)
	{
		return 1;
	}
	m_resultImageTextureID = ImGui_ImplVulkan_AddTexture(_sampler, m_resultImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


	for (auto& [key, tool]: m_tools)
	{
		if ( tool->on_resize() != 0) return 1;
	}
	return 0;
}

void SmoothieEditor::Core::on_scene_load(const std::string &settings_file)
{
	Smoothie::XML::Element __root;
	Smoothie::XML::XMLError __error;
	Smoothie::XML::build_tree_from_xml_file(settings_file, __root, __error);
	for (auto& [key, tool]: m_tools)
	{
		tool->on_scene_load(__root);
	}

}

void SmoothieEditor::Core::on_scene_save(const std::string &settings_file)
{
	Smoothie::XML::Element __root("SmoothieEditor");
	Smoothie::XML::XMLError __error;
	for (auto& [key, tool]: m_tools)
	{
		tool->on_scene_save(__root);
	}
	Smoothie::XML::build_xml_file_from_tree(__root, settings_file, __error);
}

void SmoothieEditor::Core::destroy()
{
    for (auto& [key, tool]: m_tools) {tool->destroy();}

    for (auto& [key, shader]: m_editor_shaders) {vkDestroyShaderModule(SmoothieCore::getDevice(), shader, nullptr), shader=nullptr;}
    m_editor_shaders.clear();

	vkDestroyPipeline(SmoothieCore::getDevice(), m_resultPipeline, nullptr), m_resultPipeline = nullptr;
	ImGui_ImplVulkan_RemoveTexture(m_resultImageTextureID), m_resultImageTextureID = nullptr;
	vkDestroyImageView(SmoothieCore::getDevice(), m_resultImageView, nullptr), m_resultImageView = nullptr;
	vmaDestroyImage(SmoothieCore::getVulkanMemoryAllocator(), m_resultImage, m_resultImageAllocation);
	m_resultImage = nullptr, m_resultImageAllocation = nullptr;
	ImGui_ImplVulkan_Shutdown();

}

