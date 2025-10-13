#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "ResourceManager/ResourceHandler.h"
#include <unordered_map>
#include "Core/Constants.h"


namespace Smoothie 
{ 
	//This structure represents an interface between shader inputs/outputs and C++ code.
	//Compiler will save this data inside binary shader files. It can be used to create run-time data for shaders.
	struct ShaderObjectReflection
	{
		int offset;
		ModelDescriptorDataType type;
		int size;                   // data size in bytes for a block, array size for a (non-block) object that's an array
		int index;
		int counterIndex;
		int numMembers;
		int arrayStride;            // stride of an array variable
		int topLevelArraySize;      // size of the top-level variable in a storage buffer member
		int topLevelArrayStride;    // stride of the top-level variable in a storage buffer member

		int location = -1;
		int binding = -1;
		int set = -1;

		int owning_set = -1;
		int owning_set_binding = -1;

		VkShaderStageFlagBits stages;
		std::string name;
	
		ShaderObjectReflection() : offset(-1), type(MODEL_DESCRIPTOR_DATA_TYPE_NO_TYPE), 
			size(-1), index(-1), counterIndex(-1), numMembers(-1), arrayStride(0),
			topLevelArraySize(0), topLevelArrayStride(0), stages(VK_SHADER_STAGE_ALL) {}
	};

	//Contains one shader module, its input and output data, identification name etc 
	struct ShaderModule
	{
		VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		VkShaderModule shader_module = nullptr;
		std::vector<ShaderObjectReflection> pipe_inputs;
		std::vector<ShaderObjectReflection> pipe_outputs;
		std::vector<ShaderObjectReflection> reflections;

		ShaderModule() = default;
	};

	class ShaderFile :public ResourceHandler<ShaderFile>
	{

	public:

		int create(const std::string& shaderFile) override;
		void destroy() override;
		
		struct SearchKey 
		{
			std::string name;
			VkShaderStageFlagBits stage;
			SearchKey() = default;
			SearchKey(const std::string& _name, VkShaderStageFlagBits _stage) : name(_name), stage(_stage) {};
			bool operator==(const SearchKey& other) const;
		};

		struct SearchKeyHash 
		{
			std::size_t operator()(const SearchKey& p) const;
			SearchKeyHash() = default;
		};

		std::unordered_map<SearchKey, ShaderModule, SearchKeyHash> shaders;

		ShaderFile() = default;

		int get_pipeline_data_shaders(const std::string& identifier, VkShaderStageFlagBits stage, VkPipelineShaderStageCreateInfo& createInfo) const;
		int get_pipeline_data_shaders(const std::string& identifier, VkShaderStageFlagBits stage, std::vector<VkPipelineShaderStageCreateInfo>& createInfos) const;

		int get_pipeline_data_vertex_stage_data(
			const std::string& identifier,
			std::vector<VkVertexInputBindingDescription>& vertexShaderBindingDescriptions,
			std::vector<VkVertexInputAttributeDescription>& vertexShaderAttributeDescriptions) const;

		inline const std::string& get_filepath() const { return filepath; }


	protected:
		std::string filepath;
		bool isReady = false;

	};
	
	int get_VkFormat_from_ModelDescriptorDataType(ModelDescriptorDataType type, VkFormat& result, unsigned int& format_size);

	//Parses the shader file and adds its module and name to the map. 
	int Add_system_shader(const std::string& shaderFile, std::unordered_map<std::string, VkShaderModule>& modules);
}
