#pragma once
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include <Core/Shader.h>


namespace Smoothie 
{
	//***************************************
	//Base class for all pipeline models. 
	//This should exist per-shader file.
	//***************************************
	class Pipeline_Base
	{
		unsigned int PipelineID = 0;
		mutable unsigned int useCount = 0; 
	protected:
		VkPipeline pipeline = nullptr;
		VkPipelineLayout pipeline_layout = nullptr;

		inline void setID(unsigned int ID) { PipelineID = ID; }
		inline unsigned int getUseCount() const { return useCount; }
		inline void increaseUseCount() const { useCount++; }
		inline void decreaseUseCount() const { useCount--; }

	public:

		virtual int create(const ShaderFile& shaderFile) = 0;
		virtual void destroy() = 0;

		inline unsigned int getID() const { return PipelineID; }
		inline virtual int update() { return 0; }
		
		/*inline void setRenderPass(VkRenderPass renderPass) { this->renderPass = renderPass; }
		inline VkRenderPass getRenderPass() const { return renderPass; }*/

		Pipeline_Base() = default;
	};

	//**********************************************************
	//Helper class where a bunch of pipeline states are pre-defined, primerly used for post-processing effects.
	//**********************************************************
	struct DefaultPipelineState
	{
		VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo = {};
		VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo = {};
		VkPipelineTessellationStateCreateInfo TessellationStateCreateInfo = {};
		VkPipelineViewportStateCreateInfo ViewportStateCreateInfo = {};
		VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo = {};
		VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo = {};
		VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo = {};
		VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo = {};
		
		VkPipelineColorBlendAttachmentState ColorBlendAttachmentState = {};
		VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo = {};

	public:
		inline VkPipelineVertexInputStateCreateInfo& getVertexInputStateCreateInfo()
		{
			VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			VertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
			VertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
			VertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
			VertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;
			return VertexInputStateCreateInfo;
		}

		inline VkPipelineInputAssemblyStateCreateInfo& getInputAssemblyStateCreateInfo()
		{
			InputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			InputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			InputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
			return InputAssemblyStateCreateInfo;
		}

		inline VkPipelineTessellationStateCreateInfo& getTessellationStateCreateInfo()
		{
			return TessellationStateCreateInfo;
		}

		inline VkPipelineViewportStateCreateInfo& getViewportStateCreateInfo()
		{
			ViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			ViewportStateCreateInfo.viewportCount = 1;
			ViewportStateCreateInfo.scissorCount = 1;
			return ViewportStateCreateInfo;
		}

		inline VkPipelineRasterizationStateCreateInfo& getRasterizationStateCreateInfo()
		{
			RasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			RasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
			RasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
			RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			RasterizationStateCreateInfo.depthClampEnable = false;
			RasterizationStateCreateInfo.rasterizerDiscardEnable = false;
			RasterizationStateCreateInfo.lineWidth = 1.0f;
			RasterizationStateCreateInfo.depthBiasEnable = false;
			return RasterizationStateCreateInfo;
		}

		inline VkPipelineMultisampleStateCreateInfo& getMultisampleStateCreateInfo()
		{
			MultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			MultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
			MultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			return MultisampleStateCreateInfo;
		}

		inline VkPipelineDepthStencilStateCreateInfo& getDepthStencilStateCreateInfo() 
		{
			DepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			DepthStencilStateCreateInfo.depthTestEnable = false;
			DepthStencilStateCreateInfo.depthWriteEnable = false;
			DepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			DepthStencilStateCreateInfo.depthBoundsTestEnable = false;
			DepthStencilStateCreateInfo.stencilTestEnable = false;
			return DepthStencilStateCreateInfo;
		}

		inline VkPipelineColorBlendStateCreateInfo& getColorBlendStateCreateInfo()
		{
			ColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			ColorBlendStateCreateInfo.logicOpEnable = false;
			ColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
			ColorBlendStateCreateInfo.attachmentCount = 1;
			ColorBlendStateCreateInfo.pAttachments = &getColorBlendAttachmentState();

			ColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
			ColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
			ColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
			ColorBlendStateCreateInfo.blendConstants[3] = 0.0f;
			return ColorBlendStateCreateInfo;
		}

		inline VkPipelineColorBlendAttachmentState& getColorBlendAttachmentState()
		{
			ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			ColorBlendAttachmentState.blendEnable = false;
			ColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
			return ColorBlendAttachmentState;
		}

		inline VkPipelineDynamicStateCreateInfo& getDynamicStateCreateInfo()
		{

			DynamicStateCreateInfo.dynamicStateCount = 2;
			DynamicStateCreateInfo.flags = 0;
			DynamicStateCreateInfo.pDynamicStates = dynamicStates;
			DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			DynamicStateCreateInfo.pNext = nullptr;

			return DynamicStateCreateInfo;
		}

		inline void populate_pipeline(VkGraphicsPipelineCreateInfo& pipelineInfo) 
		{
			pipelineInfo.pVertexInputState = &getVertexInputStateCreateInfo();
			pipelineInfo.pInputAssemblyState = &getInputAssemblyStateCreateInfo();
			pipelineInfo.pViewportState = &getViewportStateCreateInfo();
			pipelineInfo.pRasterizationState = &getRasterizationStateCreateInfo();
			pipelineInfo.pDepthStencilState = &getDepthStencilStateCreateInfo();
			pipelineInfo.pMultisampleState = &getMultisampleStateCreateInfo();
			pipelineInfo.pColorBlendState = &getColorBlendStateCreateInfo();
			pipelineInfo.pDynamicState = &getDynamicStateCreateInfo();
		}

	};

}

