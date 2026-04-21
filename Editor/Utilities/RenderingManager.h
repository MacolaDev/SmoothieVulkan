#pragma once
#include "Smoothie.h"
#include "imgui.h"

namespace SmoothieEditor 
{
	struct Swizzler 
	{
	private:
		VkImage srcImage = nullptr;
		VkImageView srcImageView = nullptr;
		VkImage dstImage = nullptr;
		VkImageView dstImageView = nullptr;
		SmoothieMath::Matrix4x4 matrix;

	public:

		VkShaderModule shader = nullptr;
		VkPipeline pipeline = nullptr;
		VkPipelineLayout pipelineLayout = nullptr;
		VkSampler sampler = nullptr;

		VkDescriptorSet descriptorSet = nullptr;
		VkDescriptorSetLayout descriptorSetLayout = nullptr;
		VkDescriptorPool descriptorPool = nullptr;


		int create();
		void draw(VkCommandBuffer commandBuffer) const;

		void update_source_image(VkImage image, VkImageView imageView);
		void update_destination_image(VkImage image, VkImageView imageView);

		inline void update_matrix(const SmoothieMath::Matrix4x4& newMatrix) { matrix = newMatrix; }

		void destroy();
	};

	//Computes Data and returns it back to host device 
	struct DeviceDataGetter
	{
		struct Data
		{
			SmoothieMath::Vector3 worldPos;
			float depth = -1.0f;
		};

		VkShaderModule shader = nullptr;

		VkSampler sampler = nullptr;

		VkImage depthImage = nullptr;
		VkImageView depthImageView = nullptr;
		
		VkDescriptorSetLayout cameraDescriptorSetLayout = nullptr;

		int create();
		void compute(VkCommandBuffer commandBuffer, const ImVec2& uv) const;
		void destroy();

		int get_current_data(Data& data) const;

		void update_depth_image(VkImage image, VkImageView view);

	private:
		VkPipeline pipeline = nullptr;
		VkPipelineLayout pipelineLayout = nullptr;
		VkDescriptorSetLayout bufferDescriptorSetLayout = nullptr;

		struct _Descriptors
		{
			VkBuffer buffer = nullptr;
			VmaAllocation allocation = nullptr;
			VkDescriptorSet descriptorSet = nullptr;
			VkDescriptorPool descriptorPool = nullptr;
		};

		_Descriptors descriptors[SMOOTHIE_MAX_FRAMES_IN_FLIGHT];
	};

	const inline std::vector<const char*> __editor_shader_files =
	{
		"shaders/Editor/swizzler.sshader",
		"shaders/Editor/utilities.sshader"
	};

	struct RenderingManager: public Smoothie::DeferredRendering::Drawing
	{
	
		VkImage resultImage = nullptr;
		VkImageView resultImageView = nullptr;
		VmaAllocation resultImageAllocation = nullptr;

		VkFramebuffer resultFramebuffer = nullptr;
		VkRenderPass resultRenderPass = nullptr;

		mutable VkDescriptorSet resultDescriptorSet_ImGuiTexture = nullptr;
		VkPipeline resultingPipeline = nullptr; //Can't use the presentPipeline for drawing, since it uses different render pass
		
		Swizzler swizzler;
		DeviceDataGetter data_getter;

		int create_imgui_data();
		void destroy_imgui_data();

		void draw(VkCommandBuffer commandBuffer, unsigned int FrameID) const override;
		int resize_callback() override;

		VkShaderModule getEditorShader(const std::string& name) const;

		void update_swizzler(int index);

	protected:
		std::unordered_map<std::string, VkShaderModule> editor_shaders;
	};
}