#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <array>

#include "Core/Drawing.h"
#include "Core/Shader.h"

#include "Effects/DeferredPasses.h"
#include "Effects/DeferredPipeline.h"
#include "Effects/Deferred_Descriptors.h"
#include "Effects/Deferred_Lighting.h"
#include "Effects/Skybox.h"
#include "Effects/Bloom.h"
#include "Effects/SSAO.h"

namespace Smoothie
{

	namespace DeferredRendering
	{
		//System shaders used by the engine.
		const inline std::vector<const char*> __system_shader_files = 
		{
			"shaders/standard/pbs_maps.sshader",
			"shaders/standard/vertex_default.sshader",
			"shaders/post-processing/bloom.sshader",
			"shaders/post-processing/god_rays.sshader",
			"shaders/post-processing/present.sshader",
			"shaders/lighting/pbr_render.sshader",
			"shaders/lighting/SSAO.sshader",
			"shaders/atmosphere/skybox.sshader",
			"shaders/atmosphere/Sun.sshader"
		};


		class Drawing: public Drawing_Base
		{

		protected:
			gBufferPass gBuffer_Pass;
			HDRPass hdrPass;

			std::unordered_map<std::string, VkSampler> samplers;
			std::unordered_map<std::string, VkShaderModule> system_shaders;

			using PipelinesMap = std::unordered_map<std::string, std::shared_ptr<BasePipeline>>;

			std::mutex _gBufferMutex;
			PipelinesMap gBufferPipelines;

			std::mutex _HDRMutex;
			PipelinesMap HDRPipelines;

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

			virtual int create_system_shaders(const std::vector<const char*>& __shader_files);
			virtual void destroy_system_shaders();

		public:
			int create() override;
			void draw(VkCommandBuffer commandBuffer, unsigned int frame) const override;
			int resize_callback() override;

			void destroy() override;

			
			VkRenderPass getRenderPassGBuffer() const { return gBuffer_Pass.get_render_pass(); }
			VkRenderPass getRenderPassHDR() const { return hdrPass.get_render_pass(); }
			VkRenderPass getLightingPass() const { return lighting_renderPass.get_render_pass(); }

			VkDescriptorSet getDescriptorSet() const;
			VkDescriptorSetLayout getDescriptorSetLayout(int index) const { return descriptors[index].getDescriptrotSetLayout(); }
			VkDescriptorPool getDescriptorPool(int index) const { return descriptors[index].getDescriptrotPool(); }

			VkSampler getSampler(const std::string& name) const;
			VkShaderModule getSystemShaderModule(const std::string& identifier) const;

			virtual BasePipeline* get_or_create_pipeline(const std::string& filePath);

			int add_Model_to_rendering(const std::string& shaderFile, StandardModel& model);
			void remove_Model_from_rendering(const std::string& shaderFile, const StandardModel& model);


			inline const DrawingDescriptorSetData& getDescriptorData() const { return descriptorSetData; }
			inline void setDescriptorData(const DrawingDescriptorSetData& data) { descriptorSetData = data; }

			//Updates new descriptorSetData data to the GPU buffer
			void update_descriptorSet();

			
			inline const decltype(gBufferPipelines)& getPipelines_gBuffer() const { return gBufferPipelines; }
			inline void addPipeline_gBuffer(const std::string& file, std::shared_ptr<BasePipeline> pipe) 
			{
				std::lock_guard<std::mutex> lock(_gBufferMutex);
				if (gBufferPipelines.find(file) != gBufferPipelines.end()) return;
				gBufferPipelines[file] = pipe;
			}

			inline const decltype(HDRPipelines)& getPipelines_HDR() const { return HDRPipelines; }
			inline void addPipeline_HDR(const std::string& file, std::shared_ptr<BasePipeline> pipe)
			{
				std::lock_guard<std::mutex> lock(_HDRMutex);
				if (HDRPipelines.find(file) != HDRPipelines.end()) return;
				HDRPipelines[file] = pipe;
			}

			Drawing() = default;
		};

	}
}


