//
// Created by macola on 6/22/26.
//

#include "Pipe_Shader.h"

#include <utility>
#include <queue>

#include "../engine/engine-core.h"

#include "slang/slang.h"
#include "slang/slang-com-ptr.h"


using namespace slang;
using namespace Smoothie;

using Shader_Type_Base_ptr = std::shared_ptr<Shader_Type_Base>;

namespace
{
    struct ReflectionBuilder
    {
        std::unordered_map<std::string, Shader_Type_Base_ptr>& m_Types;
        explicit ReflectionBuilder(
            std::unordered_map<std::string, Shader_Type_Base_ptr>& types
            ) : m_Types(types){}

        void process_global_variable(slang::VariableLayoutReflection *layout, Smoothie::Variable_Global& variable) const;

        void process_type(slang::TypeLayoutReflection *layout) const;

        void process_entry_point(EntryPointReflection* layout, Shader_EntryPoint& entry);

    };
}

void ReflectionBuilder::process_global_variable(slang::VariableLayoutReflection *layout, Smoothie::Variable_Global& variable) const
{
    assert(layout != nullptr);
    if (layout == nullptr) return;

    variable.set = layout->getBindingSpace();
    variable.binding = layout->getBindingIndex();
    variable.stage = 0;
    variable.flags = Shader_Variable_Flags::None;
    variable.name = (layout->getName() == nullptr) ? std::string() : layout->getName();

    auto* _type = layout->getTypeLayout()->getType()->getElementType();

    assert(_type != nullptr);
    if (_type == nullptr) return;

    Slang::ComPtr<IBlob> _name;
    _type->getFullName(_name.writeRef());
    variable.type.resize(_name->getBufferSize());
    variable.type = static_cast<const char *>(_name->getBufferPointer());

    process_type(layout->getTypeLayout()->getElementTypeLayout());
}

static Shader_Types_Scalar get_scalar_type(TypeReflection::ScalarType scalar_type)
{
    Shader_Types_Scalar _scalar;
    switch (scalar_type)
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
    return _scalar;
}

static Shader_Type_ResourceAccess get_resource_access(SlangResourceAccess rss_access)
{
    Shader_Type_ResourceAccess _access;
    switch (rss_access)
    {
        case SLANG_RESOURCE_ACCESS_NONE:
            _access = Shader_Type_ResourceAccess::None;
            break;
        case SLANG_RESOURCE_ACCESS_READ:
            _access = Shader_Type_ResourceAccess::Read;
            break;
        case SLANG_RESOURCE_ACCESS_WRITE:
            _access = Shader_Type_ResourceAccess::Write;
            break;
        case SLANG_RESOURCE_ACCESS_READ_WRITE:
            _access = Shader_Type_ResourceAccess::ReadWrite;
            break;
        case SLANG_RESOURCE_ACCESS_APPEND:
            _access = Shader_Type_ResourceAccess::Append;
            break;

        default:
            _access = Shader_Type_ResourceAccess::Unknown;
            break;
    }
    return _access;
}

static Shader_Type_UniformSubclass get_subclass(TypeLayoutReflection* layout)
{
    Shader_Type_UniformSubclass _subclass;
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
    return _subclass;
}

static void get_resource_shape(slang::TypeLayoutReflection *layout, Shader_Type_Resource_Shape& shape, Shader_Type_Resource_ShapeFlags& shape_flags)
{
    auto _shape = layout->getResourceShape();
    switch (_shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
    {
        case SLANG_RESOURCE_NONE:
            shape = Shader_Type_Resource_Shape::None;
            break;
        case SLANG_TEXTURE_1D:
            shape = Shader_Type_Resource_Shape::Texture1D;
            break;
        case SLANG_TEXTURE_2D:
            shape = Shader_Type_Resource_Shape::Texture2D;
            break;
        case SLANG_TEXTURE_3D:
            shape = Shader_Type_Resource_Shape::Texture3D;
            break;
        case SLANG_TEXTURE_CUBE:
            shape = Shader_Type_Resource_Shape::TextureCube;
            break;
        case SLANG_STRUCTURED_BUFFER:
            shape = Shader_Type_Resource_Shape::StructuredBuffer;
            break;
        case SLANG_TEXTURE_BUFFER:
            shape = Shader_Type_Resource_Shape::TextureBuffer;
            break;
        default:
            shape = Shader_Type_Resource_Shape::Unknown;
            break;
    }

    if (shape == Shader_Type_Resource_Shape::None)
    {
        switch (layout->getKind())
        {
            case TypeReflection::Kind::SamplerState:
                shape = Shader_Type_Resource_Shape::SamplerState;
                break;

            case TypeReflection::Kind::ConstantBuffer:
                shape = Shader_Type_Resource_Shape::ConstantBuffer;
                break;

            default:
                shape = Shader_Type_Resource_Shape::Unknown;
                break;
        }
    }

    switch (_shape & SLANG_RESOURCE_EXT_SHAPE_MASK)
    {
        case SLANG_TEXTURE_ARRAY_FLAG:
            shape_flags = Shader_Type_Resource_ShapeFlags::Array;
            break;
        case SLANG_TEXTURE_MULTISAMPLE_FLAG:
            shape_flags = Shader_Type_Resource_ShapeFlags::Multisample;
            break;
        case SLANG_TEXTURE_COMBINED_FLAG:
            shape_flags = Shader_Type_Resource_ShapeFlags::Combined;
            break;
        default:
            shape_flags = Shader_Type_Resource_ShapeFlags::None;
            break;
    }
}

namespace
{
    struct WorkItem
    {
        std::string name;
        slang::TypeLayoutReflection* layout;
    };
}

void ReflectionBuilder::process_type(slang::TypeLayoutReflection *layout) const
{
    assert(layout != nullptr);
    if (layout == nullptr) return;

    auto* _type = layout->getType();
    if (_type == nullptr) return;
    std::string _full_name;

    Slang::ComPtr<IBlob> _name;
    _type->getFullName(_name.writeRef());
    _full_name.resize(_name->getBufferSize());
    _full_name = static_cast<const char *>(_name->getBufferPointer());

    if (m_Types.find(_full_name) != m_Types.end()) return;

    std::queue<WorkItem> _work_items;
    const WorkItem _work_item = {_full_name, layout};
    _work_items.push(_work_item);

    auto f_process_variable = [&_work_items](slang::VariableLayoutReflection *layout, Smoothie::Variable &variable)
    {
        assert(layout != nullptr);
        if (layout == nullptr) return;

        auto layout_name_ = layout->getName();

        variable.location_offset_in = layout->getOffset(ParameterCategory::VaryingInput);
        variable.location_offset_out = layout->getOffset(ParameterCategory::VaryingOutput);
        variable.bindings_offset = layout->getOffset(ParameterCategory::DescriptorTableSlot);
        variable.uniform_offset = layout->getOffset(ParameterCategory::Uniform);
        variable.flags = Shader_Variable_Flags::None;
        variable.name = (layout->getName() == nullptr) ? std::string() : layout->getName();

        auto* _type = layout->getTypeLayout()->getType();
        assert(_type != nullptr);
        if (_type == nullptr) return;


        Slang::ComPtr<IBlob> _name;
        _type->getFullName(_name.writeRef());
        variable.type.resize(_name->getBufferSize());
        variable.type = static_cast<const char *>(_name->getBufferPointer());
        _work_items.push({variable.type, layout->getTypeLayout()});
    };

    auto f_process_struct = [f_process_variable](slang::TypeLayoutReflection *layout, Smoothie::Shader_Type_Struct& newType)
    {
        assert(layout != nullptr);
        if (layout == nullptr) return;

        const unsigned int _field_count = layout->getFieldCount();
        newType.members.resize(_field_count);

        for (int field = 0; field < _field_count; field++)
        {
            auto* _field = layout->getFieldByIndex(field);
            f_process_variable(_field, newType.members[field]);
        }
    };

    auto f_process_uniform = [&_work_items](slang::TypeLayoutReflection* layout, Smoothie::Shader_Type_Uniform& newType)
    {
        assert(layout != nullptr);
        if (layout == nullptr) return;

        newType.builtinType = get_scalar_type(layout->getScalarType());

        auto* _element_type = layout->getElementTypeLayout();
        if (_element_type == nullptr)
        {
            newType.userType = "";
        }else
        {
            Slang::ComPtr<IBlob> _name;
            _element_type->getType()->getFullName(_name.writeRef());

            std::string _full_name;
            _full_name.resize(_name->getBufferSize());
            _full_name = static_cast<const char *>(_name->getBufferPointer());
            newType.userType = _full_name;

            _work_items.push({_full_name, _element_type});
        }

        newType.subclass = get_subclass(layout);
        newType.count = layout->getElementCount();
        newType.sizeX = layout->getColumnCount();
        newType.sizeY = layout->getRowCount();

    };

    auto f_process_resource = [&_work_items](slang::TypeLayoutReflection *layout, Smoothie::Shader_Type_Resource& newType)
    {
        assert(layout != nullptr);
        if (layout == nullptr) return;

        newType.access = get_resource_access(layout->getResourceAccess());
        get_resource_shape(layout, newType.shape, newType.shape_flags);

        auto* _result_type = layout->getResourceResultType();
        if (_result_type == nullptr)
        {
            newType.result_type = std::string();
            return;
        }

        Slang::ComPtr<IBlob> _name;
        _result_type->getFullName(_name.writeRef());

        std::string _full_name;
        _full_name.resize(_name->getBufferSize());
        _full_name = static_cast<const char *>(_name->getBufferPointer());

        newType.result_type = _full_name;
    };

    while (!_work_items.empty())
    {
        const auto& _new_work_item = _work_items.front();

        if (m_Types.find(_new_work_item.name) != m_Types.end())
        {
            _work_items.pop();
            continue;
        }

        if (_new_work_item.layout == nullptr) continue;

        switch (_new_work_item.layout->getKind())
        {
            case TypeReflection::Kind::Array:
            case TypeReflection::Kind::Matrix:
            case TypeReflection::Kind::Vector:
            case TypeReflection::Kind::Scalar:
            {
                Smoothie::Shader_Type_Uniform _uniform;
                f_process_uniform(_new_work_item.layout, _uniform);
                m_Types.insert({_new_work_item.name, std::make_shared<Shader_Type_Uniform>(_uniform)});
            }break;


            case TypeReflection::Kind::Struct:
            {

                Smoothie::Shader_Type_Struct _new_type;
                f_process_struct(_new_work_item.layout, _new_type);
                m_Types.insert({_new_work_item.name, std::make_shared<Shader_Type_Struct>(_new_type)});
            }break;

            case TypeReflection::Kind::SamplerState:
            case TypeReflection::Kind::ConstantBuffer:
            case TypeReflection::Kind::Resource:
            {
                Smoothie::Shader_Type_Resource _resource;
                f_process_resource(_new_work_item.layout, _resource);
                m_Types.insert({_new_work_item.name, std::make_shared<Smoothie::Shader_Type_Resource>(_resource)});
            } break;

            default:
            {
                m_Types.insert({_new_work_item.name, std::make_shared<Shader_Type_Base>(Shader_Type_Kind::Unknown)});
            }break;
        }


        _work_items.pop();
    }

}

static VkShaderStageFlags get_stage_from_slang(SlangStage _stage)
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
            return VK_SHADER_STAGE_ALL; //Unsupported shaders by the compiler so far :)
    }
}

void ReflectionBuilder::process_entry_point(EntryPointReflection* layout, Shader_EntryPoint& entry)
{
    const auto* _entry_point_function = layout->getFunction();
    assert(_entry_point_function != nullptr);

    entry.flags = Smoothie::Shader_EntryPoint_Flags::None;
    entry.stage = get_stage_from_slang(layout->getStage());

    //Pipeline
    entry.pipeline = "wasd";


    auto f_process_variable = [this](slang::VariableLayoutReflection *var_layout, Smoothie::Variable &variable)
    {
        assert(var_layout != nullptr);
        if (var_layout == nullptr) return;

        variable.location_offset_in = var_layout->getOffset(ParameterCategory::VaryingInput);
        variable.location_offset_out = var_layout->getOffset(ParameterCategory::VaryingOutput);
        variable.bindings_offset = var_layout->getOffset(ParameterCategory::DescriptorTableSlot);
        variable.uniform_offset = var_layout->getOffset(ParameterCategory::Uniform);
        variable.flags = Shader_Variable_Flags::None;
        variable.name = (var_layout->getName() == nullptr) ? std::string() : var_layout->getName();

        auto* _type = var_layout->getTypeLayout()->getType();
        assert(_type != nullptr);
        if (_type == nullptr) return;


        Slang::ComPtr<IBlob> _name;
        _type->getFullName(_name.writeRef());
        variable.type.resize(_name->getBufferSize());
        variable.type = static_cast<const char *>(_name->getBufferPointer());
        process_type(var_layout->getTypeLayout());
    };


    //Get result type
    f_process_variable(layout->getResultVarLayout(), entry.returnVariable);

    entry.name = layout->getName();

    //Get input types
    entry.inputVariables.resize(layout->getParameterCount());
    for (int index = 0; index < layout->getParameterCount(); index++)
    {
        auto& _inputType = entry.inputVariables[index];
        auto* _parameter = layout->getParameterByIndex(index);
        f_process_variable(_parameter, _inputType);
    }
}

void Smoothie::AssetPipeline::Pipe_Shader::process(
    const std::string &input_file,
    const std::string &output_file,
    const std::vector<std::string> &processing_options)
{

    Slang::ComPtr<IGlobalSession> _global_session;
    SlangGlobalSessionDesc desc = {};
    desc.enableGLSL = false;
    if (createGlobalSession(&desc, _global_session.writeRef()) != 0)
    {
        fmt::print("Failed to create global session.");
        return;
    }

    SessionDesc _session_description;
    _session_description.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
    _session_description.skipSPIRVValidation = false;
    const char* _search_path[] = {"."};
    _session_description.searchPathCount = 1;
    _session_description.searchPaths = _search_path;

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
    {   {CompilerOptionName::Language, _language },
       {CompilerOptionName::Optimization, _optimization },
       {CompilerOptionName::DebugInformation, _debugging_information },
       {CompilerOptionName::DebugInformationFormat, _debugging_information_format },
       {CompilerOptionName::VulkanUseEntryPointName,_entryPointName}
    };


    _session_description.compilerOptionEntryCount = _compiler_options.size();
    _session_description.compilerOptionEntries = _compiler_options.data();
    TargetDesc _targetDesc;
    _targetDesc.profile = _global_session->findProfile("spirv_1_5");
    _targetDesc.format = SLANG_SPIRV;
    _session_description.targetCount = 1;
    _session_description.targets = &_targetDesc;

    Slang::ComPtr<ISession> session;
    if (_global_session->createSession(_session_description, session.writeRef()) != 0)
    {
        fmt::print("Failed to create session.");
        return;
    }


    //Load source file
    Slang::ComPtr<IBlob> diagnostics;
    IModule* module = session->loadModule(input_file.c_str(), diagnostics.writeRef());
    if(diagnostics)
    {
        fprintf(stderr, "%s\n", (const char*) diagnostics->getBufferPointer());
        return;
    }


    std::vector<slang::IComponentType*> _compiler_components;

    //Get entry points to compile
    for (int i = 0; i < module->getDefinedEntryPointCount(); i++)
    {
        Slang::ComPtr<slang::IEntryPoint> _entry_point;
        module->getDefinedEntryPoint(i, _entry_point.writeRef());
        _compiler_components.push_back(_entry_point);
    }

    //Get main module
    _compiler_components.push_back(module);


    //Compile program
    Slang::ComPtr<slang::IComponentType> composedProgram;
    session->createCompositeComponentType(
        _compiler_components.data(),
        _compiler_components.size(),
        composedProgram.writeRef(),
        diagnostics.writeRef());
    if(diagnostics)
    {
        fprintf(stderr, "%s\n", (const char*) diagnostics->getBufferPointer());
        return;
    }


    //Link program
    Slang::ComPtr<IComponentType> linkedProgram;
    composedProgram->link(linkedProgram.writeRef(), diagnostics.writeRef());
    if(diagnostics)
    {
        fprintf(stderr, "%s\n", (const char*) diagnostics->getBufferPointer());
        return;
    }



    //Get SPIR-V
    Slang::ComPtr<IBlob> _code;
    linkedProgram->getTargetCode(0, _code.writeRef(), diagnostics.writeRef());
    if(diagnostics)
    {
        fprintf(stderr, "%s\n", (const char*) diagnostics->getBufferPointer());
        return;
    }


    //File data
    std::vector<unsigned int> SPIR_V_code;
    std::vector<Shader_EntryPoint> entry_points;
    std::vector<Smoothie::Variable_Global> global_variables;
    std::unordered_map<std::string, Shader_Type_Base_ptr> types;

    auto _reflection_builder = ReflectionBuilder(types);

    //Copy SPIR-V
    SPIR_V_code.resize(_code->getBufferSize() / sizeof(unsigned int));
    std::memcpy(SPIR_V_code.data(), _code->getBufferPointer(), _code->getBufferSize());

    auto* _layout = linkedProgram->getLayout();
    auto* _global_parameters = _layout->getGlobalParamsVarLayout();
    auto* _global_type_layout = _global_parameters->getTypeLayout();

    //Get global variables
    global_variables.resize(_global_type_layout->getFieldCount());
    for (unsigned int index = 0; index < _global_type_layout->getFieldCount(); index++)
    {
        _reflection_builder.process_global_variable(_global_type_layout->getFieldByIndex(index), global_variables[index]);
    }

    //Get entry points
    entry_points.resize(_layout->getEntryPointCount());
    for (int i = 0; i < _layout->getEntryPointCount(); i++)
    {
        auto* _entry_point = _layout->getEntryPointByIndex(i);
        _reflection_builder.process_entry_point(_entry_point, entry_points[i]);
    }


    std::ofstream _output(output_file);

    auto f_write_string = [&_output](const std::string& string)
    {
        const unsigned int _str_len = string.length();
        _output.write(reinterpret_cast<const std::ostream::char_type *>(&_str_len), sizeof(_str_len));
        _output.write(string.data(), _str_len * sizeof(char));
    };


    auto f_write_variable = [&_output, &f_write_string](const Variable& variable)
    {
        _output.write(reinterpret_cast<const std::ostream::char_type *>(&variable.location_offset_in), sizeof(variable.location_offset_in));
        _output.write(reinterpret_cast<const std::ostream::char_type *>(&variable.location_offset_out), sizeof(variable.location_offset_out));
        _output.write(reinterpret_cast<const std::ostream::char_type *>(&variable.uniform_offset), sizeof(variable.uniform_offset));
        _output.write(reinterpret_cast<const std::ostream::char_type *>(&variable.bindings_offset), sizeof(variable.bindings_offset));
        _output.write(reinterpret_cast<const std::ostream::char_type *>(&variable.flags), sizeof(variable.flags));
        f_write_string(variable.name);
        f_write_string(variable.type);
    };



    //Write magic number
    _output.write(Smoothie::ShaderFile::s_Magic, sizeof(Smoothie::ShaderFile::s_Magic));

    //Write spir-v
    const unsigned int _spir_v_size = SPIR_V_code.size();
    _output.write(reinterpret_cast<const std::ostream::char_type *>(&_spir_v_size), sizeof(_spir_v_size));
    _output.write(reinterpret_cast<const std::ostream::char_type *>(SPIR_V_code.data()), SPIR_V_code.size() * sizeof(_spir_v_size));

    //Write entry points
    const unsigned int _entry_point_size = entry_points.size();
    _output.write(reinterpret_cast<const std::ostream::char_type *>(&_entry_point_size), sizeof(_entry_point_size));
    for (unsigned int i = 0; i < _entry_point_size; i++)
    {
        const auto& _entryPoint = entry_points[i];

        _output.write(reinterpret_cast<const std::ostream::char_type *>(&_entryPoint.flags), sizeof(_entryPoint.flags));
        _output.write(reinterpret_cast<const std::ostream::char_type *>(&_entryPoint.stage), sizeof(_entryPoint.stage));

        f_write_string(_entryPoint.pipeline);
        f_write_variable(_entryPoint.returnVariable);
        f_write_string(_entryPoint.name);

        const unsigned int _inputVariables_size = _entryPoint.inputVariables.size();
        _output.write(reinterpret_cast<const std::ostream::char_type *>(&_inputVariables_size), sizeof(_inputVariables_size));

        for (unsigned int k = 0; k < _inputVariables_size; k++)
        {
            f_write_variable(_entryPoint.inputVariables[k]);
        }

    }

    //Write global variables
    const unsigned int _global_variables_size = global_variables.size();
    _output.write(reinterpret_cast<const std::ostream::char_type *>(&_global_variables_size), sizeof(_global_variables_size));
    for (unsigned int i = 0; i < _global_variables_size; i++)
    {
        const auto& _variable = global_variables[i];
        _output.write(reinterpret_cast<const std::ostream::char_type *>(&_variable.set), sizeof(_variable.set));
        _output.write(reinterpret_cast<const std::ostream::char_type *>(&_variable.binding), sizeof(_variable.binding));
        _output.write(reinterpret_cast<const std::ostream::char_type *>(&_variable.stage), sizeof(_variable.stage));
        _output.write(reinterpret_cast<const std::ostream::char_type *>(&_variable.flags), sizeof(_variable.flags));
        f_write_string(_variable.name);
        f_write_string(_variable.type);
    }


    //Write types
    const unsigned int _types_size = types.size();
	_output.write(reinterpret_cast<const std::ostream::char_type *>(&_types_size), sizeof(_types_size));
	for (const auto& [key, type]: types)
	{
		const auto* _type = type.get();
		assert(_type != nullptr);

		_output.write(reinterpret_cast<const std::ostream::char_type *>(&_type->kind), sizeof(_type->kind));
		f_write_string(key);

		switch (_type->kind)
		{

			case Shader_Type_Kind::Uniform:
			{
				const auto* _casted_type = dynamic_cast<const Shader_Type_Uniform*>(_type);

				assert(_casted_type != nullptr);
                if (_casted_type == nullptr) continue;

				_output.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->builtinType), sizeof(_casted_type->builtinType));
				f_write_string(_casted_type->userType);
				_output.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->subclass), sizeof(_casted_type->subclass));
				_output.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->sizeX), sizeof(_casted_type->sizeX));
				_output.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->sizeY), sizeof(_casted_type->sizeY));
			    _output.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->count), sizeof(_casted_type->count));
			}break;

			case Shader_Type_Kind::Resource:
			{
				const auto* _casted_type = dynamic_cast<const Shader_Type_Resource*>(_type);
				assert(_casted_type != nullptr);
			    if (_casted_type == nullptr) continue;


				_output.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->flags), sizeof(_casted_type->flags));
				_output.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->access), sizeof(_casted_type->access));
				_output.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->shape), sizeof(_casted_type->shape));
				_output.write(reinterpret_cast<const std::ostream::char_type *>(&_casted_type->shape_flags), sizeof(_casted_type->shape_flags));
				f_write_string(_casted_type->result_type);
			}break;

			case Shader_Type_Kind::Struct:
			{
				const auto* _casted_type = dynamic_cast<const Shader_Type_Struct*>(_type);
				assert(_casted_type != nullptr);
			    if (_casted_type == nullptr) continue;

				unsigned int _members_size = _casted_type->members.size();
				_output.write(reinterpret_cast<const std::ostream::char_type *>(&_members_size), sizeof(_members_size));
				for (unsigned int k = 0; k < _members_size; k++)
				{
					const auto& _member = _casted_type->members[k];
					f_write_variable(_member);
				}

			}break;


			default:
				continue;
		}

	}

    _output.close();

}

std::string_view Smoothie::AssetPipeline::Pipe_Shader::help_message() const noexcept
{
    return "";
}
