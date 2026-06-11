#pragma once
#include "Core/Common.h"

#include "Core/Drawing.h"
#include "Core/Shader.h"

#include "Effects/Deferred_Scene.h"

#include "Utilities/Textures.h"
#include "Utilities/Buffers.h"

#include "Pipelines/Base.h"
#include "Pipelines/Environment/Sky.h"

#include "screen-space-effects/Lighting/Light_Global.h"

#include "Helpers/Cubemaps.h"

#include "Effects/Deferred_Lighting.h"
#include "Effects/Skybox.h"
#include "Effects/Bloom.h"
#include "Effects/SSAO.h"

namespace Smoothie
{

	namespace DeferredRendering
	{
		class Drawing: public Drawing_Base
		{

		protected:

		    mutable Pipeline_CommandBuffers m_CurrentDrawingBuffer;

			std::vector<Pipeline_CommandBuffers> m_DrawingBuffers;
		    virtual int create_command_buffers();
		    virtual void destroy_command_buffers();
		    virtual void begin_command_buffers(unsigned int frame) const;
		    virtual void end_command_buffers() const;


        //****************************** gBuffer Data ******************************//
		    Texture2D_Attachment m_gPosition, m_gNormal, m_gAlbedo, m_gMRAO;
		    Texture2D_Attachment m_Depth; // Also being created/destroyed etc. in gBuffer functions.
            virtual int create_gBuffer();
		    virtual void destroy_gBuffer();

		    virtual void populateInheritanceRenderingInfo_gBuffer(VkCommandBufferInheritanceRenderingInfo& inheritance_info) const;

		    virtual void begin_gBuffer(VkCommandBuffer commandBuffer) const;
		    virtual void end_gBuffer(VkCommandBuffer commandBuffer) const;

		public:

		    virtual void getRenderingInfo_gBuffer(VkPipelineRenderingCreateInfo& renderingInfo) const;

		    inline const Texture2D_Attachment& getAttachmentTexture_gPosition() const { return m_gPosition; }
		    inline const Texture2D_Attachment& getAttachmentTexture_gNormal() const { return m_gNormal; }
		    inline const Texture2D_Attachment& getAttachmentTexture_gAlbedo() const { return m_gAlbedo; }
		    inline const Texture2D_Attachment& getAttachmentTexture_gMRAO() const { return m_gMRAO; } //Metalic, roughness, AO
            inline const Texture2D_Attachment& getAttachmentTexture_Depth() const { return m_Depth; }

		protected:

		    //****************************** HDR Data ********************************//
		    Texture2D_Attachment m_HDR;
		    virtual int create_hdr();
		    virtual void destroy_hdr();

		    virtual void populateInheritanceRenderingInfo_HDR(VkCommandBufferInheritanceRenderingInfo& inheritance_info) const;
		    virtual void begin_hdr(VkCommandBuffer commandBuffer) const;
		    virtual void end_hdr(VkCommandBuffer commandBuffer) const;

		public:
		    inline const Texture2D_Attachment& getAttachmentTexture_HDR() const {return m_HDR;}

		    virtual void getRenderingInfo_HDR(VkPipelineRenderingCreateInfo& renderingInfo) const;


		protected:
		    //****************************** Samplers *******************************//
		    VkSampler m_Texture2DSampler = nullptr;
		    VkSampler m_ClampToEdgeLINEAR = nullptr;
		    VkSampler m_ClampToEdgeNEAREST = nullptr;
		    VkSampler m_RepeatNEAREST = nullptr;
		    virtual int create_samplers();
		    virtual void destroy_samplers();
		public:


		protected:

		    //****************************** Camera **************************************//
		    Camera m_Camera;

            Buffer_MappedUniform m_CameraBuffer;
            virtual int create_camera();
		    virtual void destroy_camera();


		public:
		    void update_camera(const Camera& camera) override;

		protected:

		    //*************************** Global descriptors *****************************//
		    VkDescriptorSetLayout m_Global_DescriptorSetLayout = nullptr;
		    VkDescriptorPool m_Global_DescriptorPool = nullptr;
		    VkDescriptorSet m_Global_DescriptorSet = nullptr;

		    std::string m_Global_DescriptorSetName = "g_GlobalData";
		    virtual int create_global_descriptor_sets();
		    virtual void destroy_global_descriptor_sets();

		    std::string m_Global_DescriptorVariableName_Attachments = "attachments";
		    std::string m_Global_DescriptorVariableName_Samplers = "samplers";
		    std::string m_Global_DescriptorVariableName_Camera = "camera";
		    std::string m_Global_DescriptorVariableName_ShadowCamera = "samplers";

		    virtual void populate_global_descriptor_set();
		public:

		    inline VkDescriptorSet getGlobalDescriptorSet() const {return m_Global_DescriptorSet;};
		    inline VkDescriptorSetLayout getGlobalDescriptorSetLayout() const { return m_Global_DescriptorSetLayout; }
		    inline VkDescriptorPool getGlobalDescriptorPool() const { return m_Global_DescriptorPool; }

		protected:

		    //*************************** Present pipeline *****************************//
		    std::string m_PresentPipeline_Shader = "shaders/post-processing/present.sshader";
		    ShaderFile m_PresentPipeline_ShaderFile;
		    std::unordered_map<std::string, WriteData_Descriptor> m_Global_DescriptorWriteData;


		    std::string m_PresentPipeline_VertexShaderEntryName = "vertex_QUAD";
		    std::string m_PresentPipeline_FragmentShaderEntryName = "fragment_PRESENT";
		    VkPipelineLayout m_PresentPipeline_Layout = nullptr;
		    VkPipeline m_PresentPipeline = nullptr;
		    virtual int create_present_pipeline();
		    virtual void destroy_present_pipeline();

		    std::string m_Present_DescriptorSetName = "g_PresentData";
		    VkDescriptorSet m_PresentPipeline_DescriptorSet = nullptr;
		    VkDescriptorSetLayout m_PresentPipeline_DescriptorSetLayout = nullptr;
		    VkDescriptorPool m_PresentPipeline_DescriptorPool = nullptr;
		    virtual int create_present_descriptor_set();
		    virtual void destroy_present_descriptor_set();

		public:
		    inline const std::string& getPresentPipelineVertexShaderEntryName() const {return m_PresentPipeline_VertexShaderEntryName;}
		    inline const std::string& getPresentPipelineFragmentShaderEntryName() const {return m_PresentPipeline_FragmentShaderEntryName;}

		    inline VkPipelineLayout getPresentPipelineLayout() const { return m_PresentPipeline_Layout; }
		    inline VkPipeline getPresentPipeline() const { return m_PresentPipeline; }
            inline VkShaderModule getPresentPipelineShaderModule() const {return m_PresentPipeline_ShaderFile.get_ShaderModule();}

		    inline VkDescriptorSet getPresentDescriptorSet() const { return m_PresentPipeline_DescriptorSet; }
            inline VkDescriptorSetLayout getPresentDescriptorSetLayout() const {return m_PresentPipeline_DescriptorSetLayout; }
		    inline VkDescriptorPool getPresentDescriptorPool() const { return m_PresentPipeline_DescriptorPool; }

		    virtual void lazy_populate_graphics_pipeline(VkGraphicsPipelineCreateInfo& PipelineCreateInfo) const;

		protected:

		    //*************************** Lighting *************************************//

		    std::string m_Lighting_Global_Shader = "shaders/lighting/global.sshader";
		    std::unique_ptr<Light_Global> m_Light_Global = std::make_unique<Light_Global>();

		    virtual int create_lighting();
		    virtual void destroy_lighting();

		public:

		protected:
		    //*************************** Models/Pipelines *****************************//
		    mutable std::mutex m_Pipelines_Model_Mutex;
		    std::unordered_map<std::string, std::shared_ptr<Pipeline_Base>> m_Pipelines_Model;

		    //*************************** Environment ***********************************//
		    std::string m_Pipeline_Sky_Shader = "shaders/environment/sky.sshader";
		    std::unique_ptr<Sky> m_Pipeline_Sky = std::make_unique<Sky>();


            virtual int create_model_pipelines();
            virtual void destroy_model_pipelines();

		    //Helpers, it will be removed
		    std::string m_Helper_HDR_CubeMaps_Shader = "shaders/lighting/cubemaps.sshader";
		    Helper_HDR_CubeMaps m_Helper_HDR_CubeMaps;

		public:
			int create() override;
			void draw(VkCommandBuffer commandBuffer, unsigned int frame) const override;
			int resize_callback() override;

			void destroy() override;

			Drawing() = default;
		};

	}
}


