#pragma once
#include "Core/RenderPass.h"
#include "Core/Effect.h"
#include "Core/Texture.h"
#include "Effects/DeferredPasses.h"
#include "Effects/Skybox.h"

namespace Smoothie 
{
	namespace DeferredRendering
	{

		class Lighting_RenderPass: public RenderPass_Base
		{
		protected:

		public:
			Attachment_Color target;

			int create() override;
			int resize_callback() override;
			void destroy() override;

			void bindPass(VkCommandBuffer commandBuffer, unsigned int FrameID) const override;
			void unbindPass(VkCommandBuffer commandBuffer, unsigned int FrameID) const override;
			
		};

		class Lighting_Base: public Effect_Base 
		{
		protected:
			VkPipeline pipeline = nullptr;
			VkPipelineLayout  pipelineLayout = nullptr;

		public:
			VkDescriptorSet gBufferImagesDescriptorSet = nullptr;
			VkDescriptorSetLayout gBufferImagesDescriptorSetLayout = nullptr;
		};

		//Calculates BRDF LUT for global illumination
		class BRDF 
		{
		protected:
			VkDescriptorSet descriptorSet = nullptr;
			VkDescriptorSetLayout descriptorSetLayout = nullptr;
			VkDescriptorPool descriptorPool = nullptr;

			VmaAllocation BRDFallocation = nullptr;
			VkImage BRDImage = nullptr;
			VkImageView BRDFImageView = nullptr;

		public:
			virtual int create();
			virtual void destroy();

			VkShaderModule vertexShader = nullptr;
			VkShaderModule fragmentShader = nullptr;

			VkSampler Texture2DModelSampler = nullptr;

			inline VkDescriptorSet getDescriptorSet() const { return descriptorSet; }
			inline VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
			inline VkDescriptorPool getDescriptorPool() const { return descriptorPool; }

			inline VkImage getImage() const { return BRDImage; }
			inline VkImageView getImageLayout() const { return BRDFImageView; }
		};

		//Class based for global indirect lighting. It contains prefilter and irradinace cubemaps.
		class IndirectLightingMaps
		{
			VmaAllocation IrradianceMapAllocation = nullptr;
			VkImage IrradianceMapImage = nullptr;
			VkImageView IrradianceMapImageView = nullptr;

			VmaAllocation PrefilterMapAllocation = nullptr;
			VkImage PrefilterMapImage = nullptr;
			VkImageView PrefilterMapImageView = nullptr;

			VkDescriptorSetLayout descriptorSetLayout = nullptr;
			VkDescriptorSet descriptorSet = nullptr;
			VkDescriptorPool descriptorPool = nullptr;

		public:

			VkShaderModule pbsVertexModule = nullptr;
			VkShaderModule irradianceMapModule = nullptr;
			VkShaderModule prefilterMapModule = nullptr;

			virtual int create();
			virtual int create_maps_from_skybox_cubemap(const DeferredRendering::SkyboxCubemapTexture& skyboxCubemap);
			virtual void destroy();

			inline VkDescriptorSet getDescriptorSet() const { return descriptorSet; }
			inline VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
			inline VkDescriptorPool getDescriptorPool() const { return descriptorPool; }

			inline VkImage getIrradianceMapImage() const { return IrradianceMapImage; }
			inline VkImageView getIrradiangeMapImageView() const { return IrradianceMapImageView; }
			inline VmaAllocation getIrradinceMapAllocation() const { return IrradianceMapAllocation; }

			inline VkImage getPrefilterMapImage() const { return PrefilterMapImage; }
			inline VkImageView getPrefilterImageView() const { return PrefilterMapImageView; }
			inline VmaAllocation getPrefilterMapAllocation() const { return PrefilterMapAllocation; }

			VkSampler Texture2DModelSampler = nullptr;
		};


		//Global illumination lighting.
		class Global_Illumination : public Lighting_Base
		{

		public:
			VkDescriptorSet BRDF_DescriptorSet = nullptr;
			VkDescriptorSetLayout BRDF_DescriptorSetLayout = nullptr;

			VkDescriptorSet IrradiancePrefilter_DescriptorSet = nullptr;
			VkDescriptorSetLayout IrradiancePrefilter_DescriptorSetLayout = nullptr;

			VkDescriptorSetLayout DrawClassSetLayout = nullptr;
			VkRenderPass renderPass;

			VkShaderModule vertexShader = nullptr;
			VkShaderModule globalIlluminationModule = nullptr;

			int create() override;
			void destroy() override;
			void draw(VkCommandBuffer commandBuffer,VkDescriptorSet drawClassSet, unsigned int ImageID) const override;
			int resize_callback() override;
		};


	}

}

