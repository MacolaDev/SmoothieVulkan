#pragma once
#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#define SMOOTHIE_ENABLE_BITMASK_OPERATORS(x) \
inline x operator|(x a, x b) { \
return static_cast<x>( \
static_cast<int>(a) | static_cast<int>(b)); \
} \
inline x operator&(x a, x b) { \
return static_cast<x>( \
static_cast<int>(a) & static_cast<int>(b)); \
} \
inline x operator^(x a, x b) { \
return static_cast<x>( \
static_cast<int>(a) & static_cast<int>(b)); \
} \


namespace Smoothie 
{

	//****************************************************** Shader Variables *************************************************//

	enum class Shader_Variable_Flags: int
	{
		None = 0
	};
	SMOOTHIE_ENABLE_BITMASK_OPERATORS(Shader_Variable_Flags);

	struct Variable
	{
		unsigned int location_offset_in;
		unsigned int location_offset_out;

		unsigned int uniform_offset;
		unsigned int bindings_offset;

		Shader_Variable_Flags flags;
		std::string name;
		std::string type;
	};

	struct Variable_Global
	{
		unsigned int set; // one set per global variable
		unsigned int binding; // offset
		VkShaderStageFlags stage;
		Shader_Variable_Flags flags = Shader_Variable_Flags::None;
		std::string name;
		std::string type;
	};

	//****************************************************** Shader Types *****************************************************//

	enum class Shader_Type_Kind: int
	{
		Unknown = 0,

		//Scalars values (int, float, bool etc.);
		//Vectors (int3, float2, double4 etc.);
		//Matrices (float3x3, int4x2, float4x4 etc.);
		//Arrays (float[5], float64_4[] etc.);
		//Anything that is being counted as byte data inside the shader.
		Uniform = 1,

		//Textures, uniform/storage buffers, samplers, etc.
		//Everything that is being counted in descriptor bindings and sets in the shader.
		Resource = 2,

		//Group of data.
		//Structure. Its size is measured in both bytes and descriptor bindings/sets.
		Struct = 3
	};

	struct Shader_Type_Base
	{
		const Shader_Type_Kind kind = Shader_Type_Kind::Unknown;
		explicit Shader_Type_Base(Shader_Type_Kind kind) : kind(kind) {}

		virtual ~Shader_Type_Base() = default;
	};


	enum class Shader_Types_Scalar: int
	{
		None = 0,
		Void = 1,
		Bool = 2,

		Int32 = 3,
		UInt32 = 4,
		Int64 = 5,
		UInt64 = 6,

		Float16 = 7,
		Float32 = 8,
		Float64 = 9,

		Int8 = 10,
		UInt8 = 11,
		Int16 = 12,
		UInt16 = 13,
	};

	enum class Shader_Type_UniformSubclass: int
	{
		None = 0,
		Scalar = 1,
		Vector = 2,
		Matrix = 3,
		Array = 4,
	};

	struct Shader_Type_Uniform: public Shader_Type_Base
	{
		Shader_Types_Scalar builtinType = Shader_Types_Scalar::None;
		std::string userType;
		Shader_Type_UniformSubclass subclass = Shader_Type_UniformSubclass::None;
		unsigned int sizeX = 0;
		unsigned int sizeY = 0;

		inline bool isArray() const {return subclass == Shader_Type_UniformSubclass::Array;}
		inline bool isMatrix() const {return subclass == Shader_Type_UniformSubclass::Matrix;}
		inline bool isVector() const {return subclass == Shader_Type_UniformSubclass::Vector;}
		inline bool isScalar() const {return subclass == Shader_Type_UniformSubclass::Scalar;}
		inline unsigned int get_ArraySize() const {return sizeX;}

		explicit Shader_Type_Uniform(): Shader_Type_Base(Shader_Type_Kind::Uniform) {}
	};

	struct Shader_Type_Struct: public Shader_Type_Base
	{
		std::vector<Variable> members;
		explicit Shader_Type_Struct(): Shader_Type_Base(Shader_Type_Kind::Struct){};
	};


	enum class Shader_Type_ResourceAccess
	{
		None = 0,
		Read = 1,
		Write = 2,
		ReadWrite = 3,
		Unknown = 4,
	};

	enum class Shader_Type_Resource_Shape
	{
		None = 0,
		Texture1D = 1,
		Texture2D = 2,
		Texture3D = 3,
		TextureCube = 4,
		TextureBuffer = 5,
		StructuredBuffer = 6,
		Unknown
	};

	enum class Shader_Type_Resource_ShapeFlags: int
	{
		None = 0,
		Array = 1 << 0,
		Multisample = 1 << 1,
	};
	SMOOTHIE_ENABLE_BITMASK_OPERATORS(Shader_Type_Resource_ShapeFlags)

	enum class Shader_Type_Resource_Flags: int
	{
		None = 0,
	};
	SMOOTHIE_ENABLE_BITMASK_OPERATORS(Shader_Type_Resource_Flags)

	struct Shader_Type_Resource: public Shader_Type_Base
	{
		Shader_Type_Resource_Flags flags = Shader_Type_Resource_Flags::None;
		Shader_Type_ResourceAccess access = Shader_Type_ResourceAccess::Unknown;
		Shader_Type_Resource_Shape shape = Shader_Type_Resource_Shape::Unknown;
		Shader_Type_Resource_ShapeFlags shape_flags = Shader_Type_Resource_ShapeFlags::None;
		std::string result_type;
		explicit Shader_Type_Resource(): Shader_Type_Base(Shader_Type_Kind::Resource){};
	};

	//****************************************************** Shader Entry points **********************************************//

	enum class Shader_EntryPoint_Flags: int
	{
		None=0,
	};
	SMOOTHIE_ENABLE_BITMASK_OPERATORS(Shader_EntryPoint_Flags)

	struct Shader_EntryPoint
	{
		Shader_EntryPoint_Flags flags = Shader_EntryPoint_Flags::None;
		VkShaderStageFlags stage; //Entry point stage.
		std::string pipeline; //Target pipeline. In slang source, this is the "TargetPipeline" attribute.
		Variable returnVariable;
		std::string name; //Entry point function name.
		std::vector<Variable> inputVariables;
	};

	//****************************************************** Shader file ******************************************************//
	enum class ShaderFile_CreateFlags: int
	{
		None = 0,
		Hold_SPIR_V = 1 << 0,
		DontCreateModule = 1 << 1,
	};
	SMOOTHIE_ENABLE_BITMASK_OPERATORS(ShaderFile_CreateFlags)

	//Main class for shader files, with reflections.
	//WARNING: Current reflection API is not supposed to reflect every possible combinations of input resources for a shader.
	//Check the description of every reflected object (like entry points, structures, global resources etc.) for additional information about their reflection.
	class ShaderFile
	{
	protected:
		static constexpr char s_Magic[4] = "301";
		ShaderFile_CreateFlags m_Flags = ShaderFile_CreateFlags::None;
		std::vector<unsigned int> m_SPIR_V_CODE;
		std::vector<Shader_EntryPoint> m_EntryPoints;
		std::vector<Variable_Global> m_GlobalVariables;

		using Shader_Type_Base_ptr = std::shared_ptr<Shader_Type_Base>;
		std::unordered_map<std::string, Shader_Type_Base_ptr> m_Types;

		std::string m_Filepath;
		VkShaderModule m_ShaderModule = nullptr;

		virtual int serialize(std::ofstream& data);
		virtual int de_serialize(std::ifstream& file);

	public:

		inline const std::vector<unsigned int>& get_SPIR_V_CODE() const { return m_SPIR_V_CODE; }
		inline const std::vector<Shader_EntryPoint>& get_EntryPoints() const { return m_EntryPoints; }
		inline const std::vector<Variable_Global>& get_GlobalVariables() const { return m_GlobalVariables; }
		inline const std::unordered_map<std::string, Shader_Type_Base_ptr>& get_Types() const { return m_Types; }

		inline ShaderFile_CreateFlags get_Flags() const { return m_Flags; }
		inline const std::string& get_Filepath() const { return m_Filepath; }

		inline VkShaderModule get_ShaderModule() const { return m_ShaderModule; }

		int create(const std::string& shaderFile);
		inline int create(const std::string& shaderFile, ShaderFile_CreateFlags flags) {m_Flags = flags; return create(shaderFile);}
		void destroy();

		ShaderFile() = default;
		virtual ~ShaderFile() = default;
	};

}
