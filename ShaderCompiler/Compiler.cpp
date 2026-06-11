//
// Created by macola on 12/30/25.
//

#include "Compiler.h"

#include "slang/slang.h"
#include "slang/slang-com-ptr.h"
#include <vulkan/vulkan.h>
#include <cstring>
#include <unistd.h>

using namespace slang;
using namespace Smoothie;

static VkShaderStageFlags _get_stage_from_slang(SlangStage _stage)
{
	switch (_stage)
	{
		case SLANG_STAGE_VERTEX:
			return VK_SHADER_STAGE_VERTEX_BIT;

		case SLANG_STAGE_HULL:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

		case SLANG_STAGE_DOMAIN:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

		case SLANG_STAGE_GEOMETRY:
			return VK_SHADER_STAGE_GEOMETRY_BIT;

		case SLANG_STAGE_FRAGMENT:
			return VK_SHADER_STAGE_FRAGMENT_BIT;

		case SLANG_STAGE_COMPUTE:
			return VK_SHADER_STAGE_COMPUTE_BIT;

		default:
			return -1; //Unsupported shaders by the compiler so far :)
	}
}

int SmoothieCompiler::CompiledShader::get_data(slang::VariableLayoutReflection *layout, Smoothie::Variable &variable)
{
	//assert(layout != nullptr);
	if (layout == nullptr) return 0;

	variable.location_offset_in = layout->getOffset(ParameterCategory::VaryingInput);
	variable.location_offset_out = layout->getOffset(ParameterCategory::VaryingOutput);
	variable.bindings_offset = layout->getOffset(ParameterCategory::DescriptorTableSlot);
	variable.uniform_offset = layout->getOffset(ParameterCategory::Uniform);

	variable.flags = Shader_Variable_Flags::None;
	variable.name = (layout->getName() == nullptr) ? std::string() : layout->getName();

	auto* _type = layout->getTypeLayout()->getType();
	//assert(_type != nullptr);

	Slang::ComPtr<IBlob> _name;
	_type->getFullName(_name.writeRef());
	variable.type.resize(_name->getBufferSize());
	variable.type = static_cast<const char *>(_name->getBufferPointer());

	get_data(layout->getTypeLayout());
	return 0;
}

int SmoothieCompiler::CompiledShader::get_data(slang::VariableLayoutReflection *layout, Smoothie::Variable_Global &variable)
{
	variable.set = layout->getBindingSpace();
	variable.binding = layout->getBindingIndex();
	variable.stage = 0;
	variable.flags = Shader_Variable_Flags::None;
	variable.name = (layout->getName() == nullptr) ? std::string() : layout->getName();

	auto* _type = layout->getTypeLayout()->getType()->getElementType();
    if (_type == nullptr)
    {
        return 1;
    }

	Slang::ComPtr<IBlob> _name;
	_type->getFullName(_name.writeRef());
	variable.type.resize(_name->getBufferSize());
	variable.type = static_cast<const char *>(_name->getBufferPointer());
	get_data(layout->getTypeLayout()->getElementTypeLayout());

	return 0;
}

int SmoothieCompiler::CompiledShader::get_data(slang::TypeLayoutReflection* layout, std::shared_ptr<Shader_Type_Uniform>& newType)
{
	if (layout == nullptr)
	{
		return 0;
	}


	Shader_Types_Scalar _scalar;
	switch (layout->getScalarType())
	{
		case TypeReflection::ScalarType::None:
			_scalar = Shader_Types_Scalar::None;
			break;

		case TypeReflection::ScalarType::Void:
			_scalar = Shader_Types_Scalar::Void;
			break;

		case TypeReflection::ScalarType::Bool:
			_scalar = Shader_Types_Scalar::Bool;
			break;

		case TypeReflection::ScalarType::Int32:
			_scalar = Shader_Types_Scalar::Int32;
			break;

		case TypeReflection::ScalarType::UInt32:
			_scalar = Shader_Types_Scalar::UInt32;
			break;

		case TypeReflection::ScalarType::Int64:
			_scalar = Shader_Types_Scalar::Int64;
			break;

		case TypeReflection::ScalarType::UInt64:
			_scalar = Shader_Types_Scalar::UInt64;
			break;

		case TypeReflection::ScalarType::Float16:
			_scalar = Shader_Types_Scalar::Float16;
			break;

		case TypeReflection::ScalarType::Float32:
			_scalar = Shader_Types_Scalar::Float32;
			break;

		case TypeReflection::ScalarType::Float64:
			_scalar = Shader_Types_Scalar::Float64;
			break;

		case TypeReflection::ScalarType::Int8:
			_scalar = Shader_Types_Scalar::Int8;
			break;

		case TypeReflection::ScalarType::UInt8:
			_scalar = Shader_Types_Scalar::UInt8;
			break;

		case TypeReflection::ScalarType::Int16:
			_scalar = Shader_Types_Scalar::Int16;
			break;

		case TypeReflection::ScalarType::UInt16:
			_scalar = Shader_Types_Scalar::UInt16;
			break;

		default:
			_scalar = Shader_Types_Scalar::Unknown;
	}
	newType->builtinType = _scalar;

	auto* _element_type = layout->getElementTypeLayout();
	if (_element_type == nullptr)
	{
		newType->userType = "";
	}else
	{
		Slang::ComPtr<IBlob> _name;
		_element_type->getType()->getFullName(_name.writeRef());

		std::string _full_name;
		_full_name.resize(_name->getBufferSize());
		_full_name = static_cast<const char *>(_name->getBufferPointer());
		newType->userType = _full_name;

		get_data(_element_type);
	}


	Shader_Type_UniformSubclass _subclass;
    auto _kind = layout->getKind();
	switch (layout->getKind())
	{
		case TypeReflection::Kind::Scalar:
			_subclass = Shader_Type_UniformSubclass::Scalar;
			break;

		case TypeReflection::Kind::Vector:
			_subclass = Shader_Type_UniformSubclass::Vector;
			break;

		case TypeReflection::Kind::Matrix:
			_subclass = Shader_Type_UniformSubclass::Matrix;
			break;

		case TypeReflection::Kind::Array:
			_subclass = Shader_Type_UniformSubclass::Array;
			break;

		default:
			_subclass = Shader_Type_UniformSubclass::Unknown;
	}
    
	newType->subclass = _subclass;
    newType->count = layout->getElementCount();
	newType->sizeX = layout->getColumnCount();
	newType->sizeY = layout->getRowCount();

	return 0;
}

int SmoothieCompiler::CompiledShader::get_data(slang::TypeLayoutReflection *layout, std::shared_ptr<Smoothie::Shader_Type_Struct> &newType)
{
	if (layout == nullptr)
	{
		return 0;
	}

	if (layout->getKind() != TypeReflection::Kind::Struct) return 0;


	auto& _members = newType->members;
	const int _field_count = layout->getFieldCount();
	_members.resize(_field_count);

	for (int field = 0; field < _field_count; field++)
	{
		auto* _field = layout->getFieldByIndex(field);
		get_data(_field, _members[field]);
	}

	return 0;
}

int SmoothieCompiler::CompiledShader::get_data(slang::TypeLayoutReflection *layout, std::shared_ptr<Smoothie::Shader_Type_Resource> &newType)
{
	if (layout == nullptr)
	{
		return 0;
	}

	switch (layout->getResourceAccess())
	{
		case SLANG_RESOURCE_ACCESS_NONE:
			newType->access = Shader_Type_ResourceAccess::None;
			break;
		case SLANG_RESOURCE_ACCESS_READ:
			newType->access = Shader_Type_ResourceAccess::Read;
			break;
		case SLANG_RESOURCE_ACCESS_WRITE:
			newType->access = Shader_Type_ResourceAccess::Write;
			break;
		case SLANG_RESOURCE_ACCESS_READ_WRITE:
			newType->access = Shader_Type_ResourceAccess::ReadWrite;
			break;
	    case SLANG_RESOURCE_ACCESS_APPEND:
	        newType->access = Shader_Type_ResourceAccess::Append;
	        break;

		default:
			newType->access = Shader_Type_ResourceAccess::Unknown;
			break;
	}


	const auto _shape = layout->getResourceShape();
	switch (_shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
	{
		case SLANG_RESOURCE_NONE:
			newType->shape = Shader_Type_Resource_Shape::None;
			break;
		case SLANG_TEXTURE_1D:
			newType->shape = Shader_Type_Resource_Shape::Texture1D;
			break;
		case SLANG_TEXTURE_2D:
			newType->shape = Shader_Type_Resource_Shape::Texture2D;
			break;
		case SLANG_TEXTURE_3D:
			newType->shape = Shader_Type_Resource_Shape::Texture3D;
			break;
		case SLANG_TEXTURE_CUBE:
			newType->shape = Shader_Type_Resource_Shape::TextureCube;
			break;
		case SLANG_STRUCTURED_BUFFER:
			newType->shape = Shader_Type_Resource_Shape::StructuredBuffer;
			break;
	    case SLANG_TEXTURE_BUFFER:
	        newType->shape = Shader_Type_Resource_Shape::TextureBuffer;
	        break;
		default:
			newType->shape = Shader_Type_Resource_Shape::Unknown;
			break;
	}

    if (newType->shape == Shader_Type_Resource_Shape::None)
    {
        switch (layout->getKind())
        {
            case TypeReflection::Kind::SamplerState:
                newType->shape = Shader_Type_Resource_Shape::SamplerState;
                break;

            case TypeReflection::Kind::ConstantBuffer:
                newType->shape = Shader_Type_Resource_Shape::ConstantBuffer;
                break;

            default:
                newType->shape = Shader_Type_Resource_Shape::Unknown;
                break;
        }
    }

    switch (_shape & SLANG_RESOURCE_EXT_SHAPE_MASK)
    {
	    case SLANG_TEXTURE_ARRAY_FLAG:
            newType->shape_flags = Shader_Type_Resource_ShapeFlags::Array;
            break;
	    case SLANG_TEXTURE_MULTISAMPLE_FLAG:
            newType->shape_flags = Shader_Type_Resource_ShapeFlags::Multisample;
            break;
	    case SLANG_TEXTURE_COMBINED_FLAG:
            newType->shape_flags = Shader_Type_Resource_ShapeFlags::Combined;
            break;
        default:
            newType->shape_flags = Shader_Type_Resource_ShapeFlags::None;
            break;
    }

	auto* _result_type = layout->getResourceResultType();
	if (_result_type == nullptr)
	{
		newType->result_type = std::string();
		return 0;
	}

	Slang::ComPtr<IBlob> _name;
	_result_type->getFullName(_name.writeRef());

	std::string _full_name;
	_full_name.resize(_name->getBufferSize());
	_full_name = static_cast<const char *>(_name->getBufferPointer());
	newType->result_type = _full_name;

	return 0;
}

int SmoothieCompiler::CompiledShader::get_data(slang::TypeLayoutReflection *layout)
{
	auto* _type = layout->getType();
	if (_type == nullptr) return 0;

	Slang::ComPtr<IBlob> _name;
	_type->getFullName(_name.writeRef());

	std::string _full_name;
	_full_name.resize(_name->getBufferSize());
	_full_name = static_cast<const char *>(_name->getBufferPointer());

	if (m_Types.find(_full_name) != m_Types.end()) return 0;


	switch (layout->getKind())
	{

		case TypeReflection::Kind::Array:
		case TypeReflection::Kind::Matrix:
		case TypeReflection::Kind::Vector:
		case TypeReflection::Kind::Scalar:
		{
			auto _new_type = std::make_shared<Shader_Type_Uniform>();
			get_data(layout, _new_type);
			m_Types[_full_name] = _new_type;
		}break;

		case TypeReflection::Kind::Struct:
		{
			auto _new_type = std::make_shared<Shader_Type_Struct>();
			get_data(layout, _new_type);
			m_Types[_full_name] = _new_type;
		}break;

	    case TypeReflection::Kind::SamplerState:
	    case TypeReflection::Kind::ConstantBuffer:
		case TypeReflection::Kind::Resource:
		{
			auto _new_type = std::make_shared<Shader_Type_Resource>();
			get_data(layout, _new_type);
			m_Types[_full_name] = _new_type;
		} break;
		default:
		{
			m_Types[_full_name] = std::make_shared<Shader_Type_Base>(Shader_Type_Kind::Unknown);
		}break;
	}


	return 0;
}

static void _write_std_string(std::ofstream &data, const std::string& str)
{
	const unsigned int _str_len = str.length();
	data.write(reinterpret_cast<const std::ostream::char_type *>(&_str_len), sizeof(_str_len));
	data.write(str.data(), _str_len * sizeof(char));
}

static void _write_Variable(std::ofstream &data, const Variable& variable)
{
	data.write(reinterpret_cast<const std::ostream::char_type *>(&variable.location_offset_in), sizeof(variable.location_offset_in));
	data.write(reinterpret_cast<const std::ostream::char_type *>(&variable.location_offset_out), sizeof(variable.location_offset_out));
	data.write(reinterpret_cast<const std::ostream::char_type *>(&variable.uniform_offset), sizeof(variable.uniform_offset));
	data.write(reinterpret_cast<const std::ostream::char_type *>(&variable.bindings_offset), sizeof(variable.bindings_offset));
	data.write(reinterpret_cast<const std::ostream::char_type *>(&variable.flags), sizeof(variable.flags));
	_write_std_string(data, variable.name);
	_write_std_string(data, variable.type);
}

static void _write_Variable_Global(std::ofstream &data, const Variable_Global& variable)
{
	data.write(reinterpret_cast<const std::ostream::char_type *>(&variable.set), sizeof(variable.set));
	data.write(reinterpret_cast<const std::ostream::char_type *>(&variable.binding), sizeof(variable.binding));
	data.write(reinterpret_cast<const std::ostream::char_type *>(&variable.stage), sizeof(variable.stage));
	data.write(reinterpret_cast<const std::ostream::char_type *>(&variable.flags), sizeof(variable.flags));
	_write_std_string(data, variable.name);
	_write_std_string(data, variable.type);
}

int SmoothieCompiler::CompiledShader::serialize(std::ofstream &data)
{
	data.write(s_Magic, sizeof(s_Magic));
	const unsigned int _spir_v_size = m_SPIR_V_CODE.size();
	data.write(reinterpret_cast<const std::ostream::char_type *>(&_spir_v_size), sizeof(_spir_v_size));
	data.write(reinterpret_cast<const std::ostream::char_type *>(m_SPIR_V_CODE.data()), m_SPIR_V_CODE.size() * sizeof(_spir_v_size));

	const unsigned int _entry_point_size = m_EntryPoints.size();
	data.write(reinterpret_cast<const std::ostream::char_type *>(&_entry_point_size), sizeof(_entry_point_size));
	for (unsigned int i = 0; i < _entry_point_size; i++)
	{
		const auto& _entryPoint = m_EntryPoints[i];

		data.write(reinterpret_cast<const std::ostream::char_type *>(&_entryPoint.flags), sizeof(_entryPoint.flags));
		data.write(reinterpret_cast<const std::ostream::char_type *>(&_entryPoint.stage), sizeof(_entryPoint.stage));

		_write_std_string(data, _entryPoint.pipeline);
		_write_Variable(data, _entryPoint.returnVariable);
		_write_std_string(data, _entryPoint.name);

		const unsigned int _inputVariables_size = _entryPoint.inputVariables.size();
		data.write(reinterpret_cast<const std::ostream::char_type *>(&_inputVariables_size), sizeof(_inputVariables_size));

		for (unsigned int k = 0; k < _inputVariables_size; k++)
		{
			_write_Variable(data, _entryPoint.inputVariables[k]);
		}

	}

	const unsigned int _global_variables_size = m_GlobalVariables.size();
	data.write(reinterpret_cast<const std::ostream::char_type *>(&_global_variables_size), sizeof(_global_variables_size));
	for (unsigned int i = 0; i < _global_variables_size; i++)
	{
		_write_Variable_Global(data, m_GlobalVariables[i]);
	}

	const unsigned int _types_size = m_Types.size();
	data.write(reinterpret_cast<const std::ostream::char_type *>(&_types_size), sizeof(_types_size));
	for (const auto& [key, type]: m_Types)
	{
		const auto* _type = type.get();
		assert(_type != nullptr);

		data.write(reinterpret_cast<const std::ostream::char_type *>(&_type->kind), sizeof(_type->kind));
		_write_std_string(data, key);

		switch (_type->kind)
		{

			case Shader_Type_Kind::Uniform:
			{
				const auto* _casted_type = dynamic_cast<const Shader_Type_Uniform*>(_type);
				assert(_casted_type != nullptr);

				data.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->builtinType), sizeof(_casted_type->builtinType));
				_write_std_string(data, _casted_type->userType);
				data.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->subclass), sizeof(_casted_type->subclass));
				data.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->sizeX), sizeof(_casted_type->sizeX));
				data.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->sizeY), sizeof(_casted_type->sizeY));
			    data.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->count), sizeof(_casted_type->count));
			}break;

			case Shader_Type_Kind::Resource:
			{
				const auto* _casted_type = dynamic_cast<const Shader_Type_Resource*>(_type);
				assert(_casted_type != nullptr);

				data.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->flags), sizeof(_casted_type->flags));
				data.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->access), sizeof(_casted_type->access));
				data.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->shape), sizeof(_casted_type->shape));
				data.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->shape_flags), sizeof(_casted_type->shape_flags));
				_write_std_string(data, _casted_type->result_type);
			}break;

			case Shader_Type_Kind::Struct:
			{
				const auto* _casted_type = dynamic_cast<const Shader_Type_Struct*>(_type);
				assert(_casted_type != nullptr);

				unsigned int _members_size = _casted_type->members.size();
				data.write(reinterpret_cast<const std::ostream::char_type *>(&_members_size), sizeof(_members_size));

				for (unsigned int k = 0; k < _members_size; k++)
				{
					const auto& _member = _casted_type->members[k];
					_write_Variable(data, _member);
				}

			}break;


			default:
				continue;
		}

	}

	return 0;
}

int SmoothieCompiler::CompiledShader::compile_shader()
{

    Slang::ComPtr<IGlobalSession> globalSession;
	SlangGlobalSessionDesc desc = {};
	desc.enableGLSL = false;
	createGlobalSession(&desc, globalSession.writeRef());

	SessionDesc sessionDesc;
	sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
	sessionDesc.skipSPIRVValidation = false;
	const char* _search_path[] = {"."};
	sessionDesc.searchPathCount = 1;
	sessionDesc.searchPaths = _search_path;
	CompilerOptionValue _language;
	_language.intValue0 = SLANG_SOURCE_LANGUAGE_SLANG;

	CompilerOptionValue _optimization;
	_optimization.intValue0 = SlangOptimizationLevel::SLANG_OPTIMIZATION_LEVEL_NONE;

	CompilerOptionValue _entryPointName;
	_entryPointName.intValue0 = true;

	CompilerOptionValue _debugging_information;
	_debugging_information.intValue0 = SlangDebugInfoLevel::SLANG_DEBUG_INFO_LEVEL_MAXIMAL;

	CompilerOptionValue _debugging_information_format;
	_debugging_information_format.intValue0 = SlangDebugInfoFormat::SLANG_DEBUG_INFO_FORMAT_DEFAULT;

	std::vector<CompilerOptionEntry> _compiler_options =
	{
	 {CompilerOptionName::Language, _language },
	{CompilerOptionName::Optimization, _optimization },
	{CompilerOptionName::DebugInformation, _debugging_information },
	{CompilerOptionName::DebugInformationFormat, _debugging_information_format },
	{CompilerOptionName::VulkanUseEntryPointName,_entryPointName}
	};


	sessionDesc.compilerOptionEntryCount = _compiler_options.size();
	sessionDesc.compilerOptionEntries = _compiler_options.data();
	TargetDesc targetDesc;
	targetDesc.profile = globalSession->findProfile("spirv_1_5");
	targetDesc.format = SLANG_SPIRV;
	sessionDesc.targetCount = 1;
	sessionDesc.targets = &targetDesc;

	Slang::ComPtr<ISession> session;
	globalSession->createSession(sessionDesc, session.writeRef());

	Slang::ComPtr<IBlob> diagnostics;
	IModule* module = session->loadModule(m_CompilerSettings.input_file.c_str(), diagnostics.writeRef());
	if(diagnostics)
	{
		fprintf(stderr, "%s\n", (const char*) diagnostics->getBufferPointer());
		return 1;
	}


	//Entry points and module
	std::vector<slang::IComponentType*> _entry_points_and_module;
	for (int i = 0; i < module->getDefinedEntryPointCount(); i++)
	{
		Slang::ComPtr<slang::IEntryPoint> _entry_point;
		module->getDefinedEntryPoint(i, _entry_point.writeRef());
		_entry_points_and_module.push_back(_entry_point);
	}
	_entry_points_and_module.push_back(module);

	Slang::ComPtr<slang::IComponentType> composedProgram;
	session->createCompositeComponentType(
			_entry_points_and_module.data(),
			_entry_points_and_module.size(),
			composedProgram.writeRef(),
			diagnostics.writeRef());
	if(diagnostics)
	{
		fprintf(stderr, "%s\n", (const char*) diagnostics->getBufferPointer());
		return 1;
	}

	Slang::ComPtr<IComponentType> linkedProgram;
	composedProgram->link(linkedProgram.writeRef(), diagnostics.writeRef());
	if(diagnostics)
	{
		fprintf(stderr, "%s\n", (const char*) diagnostics->getBufferPointer());
		return 1;
	}


	//******************************* SPIR-V *******************************//
	Slang::ComPtr<IBlob> _code;
	linkedProgram->getTargetCode(0, _code.writeRef(), diagnostics.writeRef());
	if(diagnostics)
	{
		fprintf(stderr, "%s\n", (const char*) diagnostics->getBufferPointer());
		return 1;
	}

	this->m_SPIR_V_CODE.resize(_code->getBufferSize() / sizeof(unsigned int));
	std::memcpy(m_SPIR_V_CODE.data(), _code->getBufferPointer(), _code->getBufferSize());

	//**************************************** Global variables ****************************************//
	auto* _layout = linkedProgram->getLayout();
	auto* _global_parameters = _layout->getGlobalParamsVarLayout();
	auto* _global_type_layout = _global_parameters->getTypeLayout();
	m_GlobalVariables.resize(_global_type_layout->getFieldCount());
	for (unsigned int index = 0; index < _global_type_layout->getFieldCount(); index++)
	{
		auto* _variableLayout = _global_type_layout->getFieldByIndex(index);
		auto& _global_variable = m_GlobalVariables[index];
		get_data(_variableLayout, _global_variable);
	}

	//Is variable used ?
	for (int i = 0; i < _layout->getEntryPointCount(); i++)
	{
		slang::IMetadata* entryPointMetadata;
		linkedProgram->getEntryPointMetadata(i, 0, &entryPointMetadata);
		bool isUsed = false;
		//entryPointMetadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_REGISTER_SPACE, 0, 0, isUsed);

	}

	//**************************************** Entry points ********************************************//
	m_EntryPoints.resize(_layout->getEntryPointCount());
	for (int i = 0; i < _layout->getEntryPointCount(); i++)
	{
		auto& _entry_point_reflection = m_EntryPoints[i];

		auto* _entry_point = _layout->getEntryPointByIndex(i);
		auto* _entry_point_function = _entry_point->getFunction();
		assert(_entry_point_function != nullptr);


		_entry_point_reflection.flags = Smoothie::Shader_EntryPoint_Flags::None;
		_entry_point_reflection.stage = _get_stage_from_slang(_entry_point->getStage());
		//Pipeline
		_entry_point_reflection.pipeline = "wasd";

		get_data(_entry_point->getResultVarLayout(), _entry_point_reflection.returnVariable);

		_entry_point_reflection.name = _entry_point->getName();

		_entry_point_reflection.inputVariables.resize(_entry_point->getParameterCount());
		for (int index = 0; index < _entry_point->getParameterCount(); index++)
		{
			auto& _inputType = _entry_point_reflection.inputVariables[index];
			auto* _parameter = _entry_point->getParameterByIndex(index);
			get_data(_parameter, _inputType);
		}

	}

    return 0;
}
