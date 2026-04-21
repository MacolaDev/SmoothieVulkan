#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <array>
#include <list>

#include "Core/Drawing.h"
#include "Core/Shader.h"

#include "Effects/Deferred_Scene.h"
#include "Effects/DeferredPasses.h"
#include "Effects/Deferred_Pipeline.h"
#include "Effects/Deferred_Descriptors.h"
#include "Effects/Deferred_Lighting.h"
#include "Effects/Skybox.h"
#include "Effects/Bloom.h"
#include "Effects/SSAO.h"

namespace Smoothie
{

	namespace DeferredRendering
	{
		struct Drawing_InitInfo
		{
			std::unordered_map<std::string, std::string> system_shader_files =
			{
				{"pbs_maps", "shaders/standard/pbs_maps.sshader"},
				{"hdr_to_cubemap", "shaders/standard/hdr_to_cubemap.sshader"},
				{"vertex_default", "shaders/standard/vertex_default.sshader"},
				{"bloom", "shaders/post-processing/bloom.sshader"},
				{"god_rays", "shaders/post-processing/god_rays.sshader"},
				{"present", "shaders/post-processing/present.sshader"},
				{"pbr_render", "shaders/lighting/pbr_render.sshader"},
				//{"SSAO", "shaders/lighting/SSAO.sshader"},
				{"skybox", "shaders/atmosphere/skybox.sshader"},
				{"Sun", "shaders/atmosphere/Sun.sshader"}
			};

		};

		class Drawing: public Drawing_Base
		{

		protected:
			Drawing_InitInfo m_InitInfo;

			struct DrawingCommandBuffersData
			{
				VkCommandPool commandPool = nullptr;
				VkCommandBuffer commandBuffer_GBuffer = nullptr;
				VkCommandBuffer commandBuffer_HDR = nullptr;
				VkCommandBuffer commandBuffer_Compute = nullptr;
				VkCommandBuffer commandBuffer_Shadow = nullptr;
			};

			std::vector<DrawingCommandBuffersData> m_DrawingBuffers;

			gBufferPass gBuffer_Pass;
			HDRPass hdrPass;

			std::unordered_map<std::string, VkSampler> samplers;
			std::unordered_map<std::string, VkShaderModule> m_SystemShaders;

			std::list<std::shared_ptr<BasePipeline>> m_ModelPipelines;

			std::vector<Deferred_Descriptors> descriptors;
			DrawingDescriptorSetData descriptorSetData;

			Lighting_RenderPass lighting_renderPass;
			BRDF brdf;
			Global_Illumination lighting_global;
			SkyboxCubemapTexture skyboxTexture;
			Skybox skybox;
			IndirectLightingMaps indirect_lighting_maps;

			SSAO ssao;
			Bloom bloom;

			VkPipelineLayout presentPipelineLayout = nullptr;
			VkPipeline presentPipeline = nullptr;

			VkDescriptorSet presentDescriptorSet = nullptr;
			VkDescriptorSetLayout presentDescriptorSetLayout = nullptr;
			VkDescriptorPool presentDescriptorPool = nullptr;

			virtual int create_samplers();
			virtual int create_present_pipeline();
			virtual void destroy_present_pipeline();

			virtual int create_system_shaders();
			virtual void destroy_system_shaders();

			virtual int create_command_buffers();
			virtual void destroy_command_buffers();
			virtual void begin_command_buffers(unsigned int frame) const;
			virtual void end_command_buffers(unsigned int frame) const;

		public:
			int create() override;
			void draw(VkCommandBuffer commandBuffer, unsigned int frame) const override;
			int resize_callback() override;

			void destroy() override;

			inline VkPipelineLayout getPresentPipelineLayout() const { return presentPipelineLayout; }
			inline VkPipeline getPresentPipeline() const { return presentPipeline; }
			
			VkRenderPass getRenderPassGBuffer() const { return gBuffer_Pass.get_render_pass(); }
			VkRenderPass getRenderPassHDR() const { return hdrPass.get_render_pass(); }
			VkRenderPass getLightingPass() const { return lighting_renderPass.get_render_pass(); }

			VkDescriptorSet getDescriptorSet() const;
			VkDescriptorSetLayout getDescriptorSetLayout(int index) const { return descriptors[index].getDescriptorLayout(); }
			VkDescriptorPool getDescriptorPool(int index) const { return descriptors[index].getDescriptorPool(); }

			VkDescriptorSet getPresentDescriptorSet() const { return presentDescriptorSet; }

			VkSampler getSampler(const std::string& name) const;
			VkShaderModule getSystemShaderModule(const std::string& identifier) const;

			int add_model(const std::shared_ptr<BasePipeline>& pipeline, std::shared_ptr<Deferred_Model>& model);
			void remove_model(const std::shared_ptr<BasePipeline>& pipeline, std::shared_ptr<Deferred_Model>& model);

			inline const DrawingDescriptorSetData& getDescriptorData() const { return descriptorSetData; }
			inline void setDescriptorData(const DrawingDescriptorSetData& data) { descriptorSetData = data; }

			//Updates new descriptorSetData data to the GPU buffer
			void update_descriptorSet();

			inline const gBufferPass& getGbufferPass() const {return gBuffer_Pass; }
			inline const HDRPass& getHdrPass() const {return hdrPass; }

			Drawing() = default;
		};

	}
}


