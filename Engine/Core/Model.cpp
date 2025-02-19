#include "Model.h"
#include "ResourceManager/ParseXML.h"
#include "UniformBuffers.h"

#define _SMOOTHIE_ENGINE
#include "Core/SmoothieCore.h"
#include "Core/DeferredPipeline.h"
#include "Core/RenderPass.h"
#include "Core/LoadedModels.h"
#include "Core/CameraDescriptor.h"

#include <limits>
#include <random>
#include <mutex>

using namespace Smoothie;
using namespace SmoothieMath;

static unsigned int generate_random_key() 
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<unsigned int> dist(1, std::numeric_limits<unsigned int>::max());
	return dist(gen);
}

Model::Model(const std::string& file, SmoothieMath::Matrix4x4 modelMatrix)
{
    ParseXML xmlfile = ParseXML(file);
    mesh = Mesh(xmlfile.getElement("geometryFile").textContent);
    shader = ModelShader(xmlfile.getElement("shader").textContent);
	auto normalMatrix = modelMatrix.normalMatrix();
	modelMatrices.push_back({ modelMatrix, Matrix4x4(normalMatrix) });
	properties = ShaderProperty(xmlfile.getElement("property"), modelMatrices);
	ModelID = generate_random_key();
	modelRenderPass = shader.getRenderPass();

	//Viewport
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
	
	//Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthClampEnable = false;
	rasterizer.rasterizerDiscardEnable = false;
	rasterizer.lineWidth = 1.0f;
	rasterizer.depthBiasEnable = false;

	//Depth 
	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencil{};
	pipelineDepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencil.depthTestEnable = true;
	pipelineDepthStencil.depthWriteEnable = true;
	pipelineDepthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	pipelineDepthStencil.depthBoundsTestEnable = false;
	pipelineDepthStencil.stencilTestEnable = false;

	//Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//Color blending stuff
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = false;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = false;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	VkPipelineColorBlendAttachmentState attachments[] = { colorBlendAttachment, colorBlendAttachment, colorBlendAttachment , colorBlendAttachment };
	colorBlending.attachmentCount = 4; //TODO: Fix this for non-gBuffer models
	colorBlending.pAttachments = attachments;
	
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;
	
	//Dynamic states
	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();
	
	//Pipeline layout creation info
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	
	//TODO: Update this to support additional buffer for 
	VkDescriptorSetLayout descriptorSets[] =
	{
		CameraDescriptor::descriptorSetLayout,
		StandardUnifromBuffers::buffer2.getDescriptorSetLayout(),
		properties.getDescriptorSetLayout()
	};

	pipelineLayoutInfo.setLayoutCount = 3;
	pipelineLayoutInfo.pSetLayouts = descriptorSets;
	
	vkCreatePipelineLayout(SmoothieCore::getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

	//Pipeline creation
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;


	VkPipelineShaderStageCreateInfo pipelineVertexShaderCreateInfo{};
	pipelineVertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineVertexShaderCreateInfo.pName = "main";
	pipelineVertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipelineVertexShaderCreateInfo.module = shader.vertexShaderModule;

	VkPipelineShaderStageCreateInfo pipelineFragmentShaderCreateInfo{};
	pipelineFragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineFragmentShaderCreateInfo.pName = "main";
	pipelineFragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipelineFragmentShaderCreateInfo.module = shader.fragmentShaderModule;

	VkPipelineShaderStageCreateInfo stages[2] = { pipelineVertexShaderCreateInfo , pipelineFragmentShaderCreateInfo };
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputInfo{};
	pipelineVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkVertexInputBindingDescription bindingDescription;
	mesh.vertexBufferAttributeType.get()->populateBindingDescription(bindingDescription);
	pipelineVertexInputInfo.vertexBindingDescriptionCount = 1;
	pipelineVertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

	auto vertexAttributes = mesh.vertexBufferAttributeType.get()->getVertexAttributes();
	pipelineVertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size());
	pipelineVertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();
	pipelineInfo.pVertexInputState = &pipelineVertexInputInfo;


	VkPipelineInputAssemblyStateCreateInfo pipelineIndexInputInfo{};
	pipelineIndexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineIndexInputInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineIndexInputInfo.primitiveRestartEnable = VK_FALSE;
	pipelineInfo.pInputAssemblyState = &pipelineIndexInputInfo;
	
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pDepthStencilState = &pipelineDepthStencil;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;

	if (modelRenderPass == ShaderModelRenderPass::gBuffer)
	{
		pipelineInfo.renderPass = gBufferPass::renderPass;
	}
	if (modelRenderPass == ShaderModelRenderPass::HDRPass)
	{
		pipelineInfo.renderPass = HDRPass::renderPass;
	}
	
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;

	vkCreateGraphicsPipelines(SmoothieCore::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &modelPipeline);
}

static std::mutex addToRenderingMutex;
void Smoothie::Model::addToRendering() const
{
	if (ModelID == 0) return;
	if (modelRenderPass == ShaderModelRenderPass::UNDEFINED) return;
	
	auto model = Smoothie::LoadedModels::getModel(ModelID);
	if (model == nullptr) return;
	
	std::lock_guard<std::mutex> lock(addToRenderingMutex);
	if (modelRenderPass == ShaderModelRenderPass::gBuffer) 
	{
		DeferredPipeline::PBRModels.push_back(model);
	}
	else if (modelRenderPass == ShaderModelRenderPass::HDRPass)
	{
		DeferredPipeline::HDRModels.push_back(model);
	}
}

static std::mutex removeFromRenderingMutex;
void Smoothie::Model::removeFromRendering() const
{
	if (ModelID == 0) return;
	if (modelRenderPass == ShaderModelRenderPass::UNDEFINED) return;
	auto model = Smoothie::LoadedModels::getModel(ModelID);
	if (model == nullptr) return;

	std::lock_guard<std::mutex> lock(removeFromRenderingMutex);
	
	if (modelRenderPass == ShaderModelRenderPass::gBuffer)
	{
		DeferredPipeline::PBRModels.remove(model);
	}
	else if (modelRenderPass == ShaderModelRenderPass::HDRPass)
	{
		DeferredPipeline::HDRModels.remove(model);
	}
}

void Smoothie::Model::bindAndDraw(VkCommandBuffer commandBuffer) const
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);
	
	const VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer, 0, mesh.indexType);
	
	SmoothieCore::setViewport(commandBuffer);
	SmoothieCore::setScissor(commandBuffer);

	VkDescriptorSet descriptorSets[] = 
	{
		CameraDescriptor::descriptorSet,
		StandardUnifromBuffers::buffer2.getDescriptorSet(),
		properties.getDescriptorSet()
	};

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
		 
		pipelineLayout, 0, 
		3, descriptorSets, 
		0, nullptr);

	vkCmdDrawIndexed(commandBuffer, mesh.numberOfIndices, 1, 0, 0, 0);
}

unsigned int Smoothie::Model::getModelID() const
{
	return ModelID;
}

void Smoothie::Model::destroy()
{
	removeFromRendering();
	vkDestroyPipeline(SmoothieCore::getDevice(), modelPipeline, nullptr);
	modelPipeline = nullptr;
	
	vkDestroyPipelineLayout(SmoothieCore::getDevice(), pipelineLayout, nullptr);
	pipelineLayout = nullptr;
	
	properties.destroy();
	shader.destroy();
	mesh.destroy();
	ModelID = 0;
}
