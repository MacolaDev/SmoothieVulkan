#include "Shader.h"

#include <filesystem>
#include <fstream>

#include "SmoothieCore.h"


using namespace Smoothie;

namespace
{
    using Shader_Type_Base_ptr = std::shared_ptr<Shader_Type_Base>;
}


int ShaderFile::serialize(std::ofstream& data){return 0;}

static int _read_std_string(std::ifstream &data, std::string& str)
{
	unsigned int _str_len = 0;
	if (!data.read(reinterpret_cast<std::ostream::char_type *>(&_str_len), sizeof(_str_len))) return 1;

	str.resize(_str_len);
	if (!data.read(str.data(), _str_len)) return 1;

	return 0;
}

static int _read_Variable(std::ifstream& data, Variable& variable)
{
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.location_offset_in), sizeof(variable.location_offset_in))) return 1;
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.location_offset_out), sizeof(variable.location_offset_out))) return 1;
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.uniform_offset), sizeof(variable.uniform_offset))) return 1;
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.bindings_offset), sizeof(variable.bindings_offset))) return 1;
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.flags), sizeof(variable.flags))) return 1;
	if (_read_std_string(data, variable.name) != 0) return 1;
	if (_read_std_string(data, variable.type) != 0) return 1;

	return 0;
}

static int _read_Variable_Global(std::ifstream& data, Variable_Global& variable)
{
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.set), sizeof(variable.set))) return 1;
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.binding), sizeof(variable.binding))) return 1;
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.stage), sizeof(variable.stage))) return 1;
	if (!data.read(reinterpret_cast<std::istream::char_type *>(&variable.flags), sizeof(variable.flags))) return 1;
	if (_read_std_string(data, variable.name) != 0) return 1;
	if (_read_std_string(data, variable.type) != 0) return 1;
	return 0;
}

int ShaderFile::de_serialize(std::ifstream& file)
{
	unsigned int _filePtr = 0;

	char _magic[4] = {0};
	if (!file.read(_magic, sizeof(_magic))) return 1;
	if (std::memcmp(_magic, s_Magic, sizeof(_magic)) != 0) return 1;

	unsigned int _spir_v_quads_size = 0;
	if (!file.read(reinterpret_cast<std::istream::char_type *>(&_spir_v_quads_size), sizeof(_spir_v_quads_size))) return 1;

	m_SPIR_V_CODE.resize(_spir_v_quads_size);
	if (!file.read(reinterpret_cast<std::istream::char_type *>(m_SPIR_V_CODE.data()), _spir_v_quads_size * sizeof(unsigned int))) return 1;

	unsigned int _entry_points_count = 0;
	if (!file.read(reinterpret_cast<std::istream::char_type *>(&_entry_points_count), sizeof(_entry_points_count))) return 1;

	m_EntryPoints.resize(_entry_points_count);
	for (unsigned int i = 0; i < _entry_points_count; i++)
	{
		auto& _entry_point = m_EntryPoints[i];
		if (!file.read(reinterpret_cast<std::istream::char_type *>(&_entry_point.flags), sizeof(_entry_point.flags))) return 1;
		if (!file.read(reinterpret_cast<std::istream::char_type *>(&_entry_point.stage), sizeof(_entry_point.stage))) return 1;
		if (_read_std_string(file, _entry_point.pipeline) != 0) return 1;
		if (_read_Variable(file, _entry_point.returnVariable) != 0) return 1;
		if (_read_std_string(file, _entry_point.name) != 0) return 1;

		unsigned int _inputVariables_count = 0;
		if (!file.read(reinterpret_cast<std::istream::char_type *>(&_inputVariables_count), sizeof(_inputVariables_count))) return 1;
		_entry_point.inputVariables.resize(_inputVariables_count);
		for (unsigned int k = 0; k < _inputVariables_count; k++)
		{
			if (_read_Variable(file, _entry_point.inputVariables[k]) != 0) return 1;
		}

	}


	unsigned int _global_variables_count = 0;
	if (!file.read(reinterpret_cast<std::istream::char_type *>(&_global_variables_count), sizeof(_global_variables_count))) return 1;
	m_GlobalVariables.resize(_global_variables_count);
	for (unsigned int i = 0; i < _global_variables_count; i++)
	{
		if (_read_Variable_Global(file, m_GlobalVariables[i]) != 0) return 1;
	}

	unsigned int _types_count = 0;
	m_Types.reserve(_types_count);
	if (!file.read(reinterpret_cast<std::istream::char_type *>(&_types_count), sizeof(_types_count))) return 1;
	for (int _index = 0; _index < _types_count; _index++)
	{

		Shader_Type_Kind _kind;
		std::string _name;
		if (!file.read(reinterpret_cast<std::istream::char_type *>(&_kind), sizeof(_kind))) return 1;
		if (_read_std_string(file, _name) != 0) return 1;

		switch (_kind)
		{
			case Shader_Type_Kind::Uniform:
			{
				auto _newType = std::make_shared<Shader_Type_Uniform>();
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->builtinType), sizeof(_newType->builtinType))) return 1;
				if (_read_std_string(file, _newType->userType) != 0) return 1;
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->subclass), sizeof(_newType->subclass))) return 1;
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->sizeX), sizeof(_newType->sizeX))) return 1;
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->sizeY), sizeof(_newType->sizeY))) return 1;
			    if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->count), sizeof(_newType->count))) return 1;
				m_Types.insert({std::move(_name), std::reinterpret_pointer_cast<Shader_Type_Base>(_newType)});

			}break;

			case Shader_Type_Kind::Resource:
			{
				auto _newType = std::make_shared<Shader_Type_Resource>();
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->flags), sizeof(_newType->flags))) return 1;
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->access), sizeof(_newType->access))) return 1;
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->shape), sizeof(_newType->shape))) return 1;
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_newType->shape_flags), sizeof(_newType->shape_flags))) return 1;
				if (_read_std_string(file, _newType->result_type) != 0) return 1;
				m_Types.insert({std::move(_name), std::reinterpret_pointer_cast<Shader_Type_Base>(_newType)});

			}break;

			case Shader_Type_Kind::Struct:
			{
				auto _newType = std::make_shared<Shader_Type_Struct>();
				unsigned int _struct_member_count = 0;
				if (!file.read(reinterpret_cast<std::istream::char_type *>(&_struct_member_count), sizeof(_struct_member_count))) return 1;
				_newType->members.resize(_struct_member_count);
				for (unsigned int k = 0; k < _struct_member_count; k++)
				{
					if (_read_Variable(file, _newType->members[k]) != 0) return 1;
				}

				m_Types.insert({std::move(_name), std::reinterpret_pointer_cast<Shader_Type_Base>(_newType)});
			}break;


			default:
				break;
		}

	}


	return 0;
}

static VkDescriptorType get_descriptor_type_texture(const Shader_Type_Resource& _type)
{
    VkDescriptorType _descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    if (static_cast<bool>(_type.shape_flags & Shader_Type_Resource_ShapeFlags::Combined))
    {
        _descriptor_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }else if (_type.access == Shader_Type_ResourceAccess::ReadWrite || _type.access == Shader_Type_ResourceAccess::Write)
    {
        _descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    return _descriptor_type;
}

static VkDescriptorType get_descriptor_type_texture_buffer(const Shader_Type_Resource& _type)
{
    VkDescriptorType _descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    if (_type.access == Shader_Type_ResourceAccess::ReadWrite || _type.access == Shader_Type_ResourceAccess::Write)
    {
        _descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    }
    return _descriptor_type;
}

static int get_descriptor_write(WriteData_Descriptor& data, unsigned int count, unsigned int offset, const Shader_Type_Resource& resource)
{
    data.binding = offset;
    data.count = count;

    switch (resource.shape)
    {
        case Shader_Type_Resource_Shape::Texture1D:
        case Shader_Type_Resource_Shape::Texture2D:
        case Shader_Type_Resource_Shape::Texture3D:
        case Shader_Type_Resource_Shape::TextureCube:
        {
            data.type = get_descriptor_type_texture(resource);
        }
        break;


        case Shader_Type_Resource_Shape::TextureBuffer:
        {
            data.type = get_descriptor_type_texture_buffer(resource);
        }break;


        case Shader_Type_Resource_Shape::ConstantBuffer:
        {
            data.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }break;


        case Shader_Type_Resource_Shape::SamplerState:
        {
            data.type = VK_DESCRIPTOR_TYPE_SAMPLER;
        }break;


        case Shader_Type_Resource_Shape::StructuredBuffer:
        {
            data.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        }break;

        default:
        {
            SmoothieCore::logError(SMOOTHIE_SRC_LOCATION("Unknown or unsupported format type: ") + std::to_string(static_cast<int>(resource.shape)));
            return 1;
        }break;
    }

    return 0;
}

static int get_structure_descriptor_data(const std::string&  path,
    const Shader_Type_Struct& structure,
    const std::unordered_map<std::string, Shader_Type_Base_ptr>& all_types,
    std::unordered_map<std::string, WriteData_Descriptor>& descriptor_map,
    std::unordered_map<std::string, WriteData_Buffer>& uniform_map,
    unsigned int& binding_offset, unsigned int& uniform_offset)
{

    for (const auto& _member: structure.members)
    {

        if (const auto& _type = all_types.find(_member.type); _type == all_types.end())
        {
            SmoothieCore::logError_Formated(R"({} Could not find type "{}" of a variable named "{}". )", SMOOTHIE_LOCATION, _member.type, _member.name);
            return 1;
        }

        const auto& _type = all_types.at(_member.type);
        if (_type == nullptr)
        {
            SmoothieCore::logError_Formated(R"({} Type: "{}" does not have a reflection.)", SMOOTHIE_LOCATION, _member.name);
            return 1;
        }



        switch (_type->kind)
        {
            case Shader_Type_Kind::Uniform:
            {
                assert(false);
            }break;


            case Shader_Type_Kind::Resource:
            {
                const auto& _resource = std::dynamic_pointer_cast<Shader_Type_Resource>(_type);
                if (_resource == nullptr)
                {
                    SmoothieCore::logError_Formated(R"({} Failed to cast type from "Shader_Type_Base" to "Shader_Type_Resource" for variable: {})", SMOOTHIE_LOCATION, _member.name);
                    return 1;
                }

                WriteData_Descriptor _write_data;
                if (get_descriptor_write(_write_data, 1, binding_offset, *_resource) != 0)
                {
                    SmoothieCore::logError_Formated(R"({} Unable to get data for member: {} )", SMOOTHIE_LOCATION, _member.name);
                    return 1;
                }

                descriptor_map[path + "." + _member.name] = _write_data;
                binding_offset++;
            }break;


            case Shader_Type_Kind::Struct:
            {
                const auto& _struct = std::dynamic_pointer_cast<Shader_Type_Struct>(_type);
                if (_struct == nullptr)
                {
                    SmoothieCore::logError_Formated(R"({} Failed to cast type from "Shader_Type_Base" to "Shader_Type_Struct" for variable: {}")", SMOOTHIE_LOCATION, _member.name);
                    return 1;
                }

                if (get_structure_descriptor_data(path + "." + _member.name, *_struct, all_types, descriptor_map, uniform_map, binding_offset, uniform_offset) != 0)
                {
                    SmoothieCore::logError_Formated(R"({} Unable to get reflection data for: {}")", SMOOTHIE_LOCATION,  _member.name);
                    return 1;
                }

            }break;


            default:
            {
                SmoothieCore::logError_Formated(R"({} Unknown kind of a struct for a member: {}")", SMOOTHIE_LOCATION,  _member.name);
                return 1;
            }break;
        }

    }


    return 0;
}

int ShaderFile::get_DescriptorData(
            const std::string& global_variable,
            std::unordered_map<std::string, WriteData_Descriptor>& descriptor_map,
            std::unordered_map<std::string, WriteData_Buffer>& uniform_map)
{
    Variable_Global _global_variable;
    std::size_t _index = 0;
    for (; _index < m_GlobalVariables.size(); _index++)
    {
        if (global_variable == m_GlobalVariables.at(_index).name)
        {
            _global_variable = m_GlobalVariables.at(_index);
            break;
        }
    }
    if (_index >= m_GlobalVariables.size())
    {
        SmoothieCore::logError_Formated("No global variable named: {}", global_variable);
        return 1;
    }

    if (m_Types.find(_global_variable.type) == m_Types.end())
    {
        SmoothieCore::logError_Formated("Unable to find variable with a name: {}", _global_variable.name);
        return 1;
    }

    const auto& _global_variable_type = m_Types.at(_global_variable.type);
    if (_global_variable_type == nullptr)
    {
        SmoothieCore::logError_Formated("Unknown global variable of a type: {}", _global_variable.type);
        return 1;
    }

    const auto& _global_variable_type_struct = std::dynamic_pointer_cast<Shader_Type_Struct>(_global_variable_type);
    if (_global_variable_type_struct == nullptr)
    {
        SmoothieCore::logError_Formated(R"(Type of the variable "{}" must be struct!)", _global_variable.name);
        return 1;
    }

    unsigned int _binding_offset = 0, _uniform_offset = 0;
    if (get_structure_descriptor_data(_global_variable.name, *_global_variable_type_struct, m_Types, descriptor_map, uniform_map, _binding_offset, _uniform_offset) != 0)
    {
        SmoothieCore::logError("Failed to get data!");
        return 1;
    }

    return 0;
}

static int get_resource_descriptor_data(
    const Shader_Type_Resource& _type,
    std::vector<VkDescriptorSetLayoutBinding> &bindings,
    std::vector<VkDescriptorPoolSize> &sizes,
    unsigned int dst_binding,
    VkShaderStageFlags stages,
    unsigned int _array_size = 1)
{

    if (_array_size == 0)
    {
        SmoothieCore::logError_Formated("Invalid number for descriptor count: ", _array_size);
        return 1;
    }

    switch (_type.shape)
    {
        case Shader_Type_Resource_Shape::Texture1D:
        case Shader_Type_Resource_Shape::Texture2D:
        case Shader_Type_Resource_Shape::Texture3D:
        case Shader_Type_Resource_Shape::TextureCube:
        {
            const VkDescriptorType _descriptor_type = get_descriptor_type_texture(_type);

            VkDescriptorSetLayoutBinding _binding = {};
            _binding.binding = dst_binding;
            _binding.descriptorType = _descriptor_type;
            _binding.descriptorCount = _array_size;
            _binding.stageFlags = stages;
            _binding.pImmutableSamplers = nullptr;
            bindings.push_back(_binding);

            VkDescriptorPoolSize _size = {};
            _size.type = _descriptor_type;
            _size.descriptorCount = _array_size;
            sizes.push_back(_size);
        }
        break;


        case Shader_Type_Resource_Shape::TextureBuffer:
        {
            const VkDescriptorType _descriptor_type = get_descriptor_type_texture_buffer(_type);

            VkDescriptorSetLayoutBinding _binding = {};
            _binding.binding = dst_binding;
            _binding.descriptorType = _descriptor_type;
            _binding.descriptorCount = _array_size;
            _binding.stageFlags = stages;
            _binding.pImmutableSamplers = nullptr;
            bindings.push_back(_binding);

            VkDescriptorPoolSize _size = {};
            _size.type = _descriptor_type;
            _size.descriptorCount = _array_size;
            sizes.push_back(_size);
        }break;


        case Shader_Type_Resource_Shape::ConstantBuffer:
        {
            VkDescriptorType _descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            VkDescriptorSetLayoutBinding _binding = {};
            _binding.binding = dst_binding;
            _binding.descriptorType = _descriptor_type;
            _binding.descriptorCount = _array_size;
            _binding.stageFlags = stages;
            _binding.pImmutableSamplers = nullptr;
            bindings.push_back(_binding);

            VkDescriptorPoolSize _size = {};
            _size.type = _descriptor_type;
            _size.descriptorCount = _array_size;
            sizes.push_back(_size);
        }break;


        case Shader_Type_Resource_Shape::SamplerState:
        {
            VkDescriptorType _descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLER;

            VkDescriptorSetLayoutBinding _binding = {};
            _binding.binding = dst_binding;
            _binding.descriptorType = _descriptor_type;
            _binding.descriptorCount = _array_size;
            _binding.stageFlags = stages;
            _binding.pImmutableSamplers = nullptr;
            bindings.push_back(_binding);

            VkDescriptorPoolSize _size = {};
            _size.type = _descriptor_type;
            _size.descriptorCount = _array_size;
            sizes.push_back(_size);
        }break;


        case Shader_Type_Resource_Shape::StructuredBuffer:
        {
            VkDescriptorType _descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

            VkDescriptorSetLayoutBinding _binding = {};
            _binding.binding = dst_binding;
            _binding.descriptorType = _descriptor_type;
            _binding.descriptorCount = _array_size;
            _binding.stageFlags = stages;
            _binding.pImmutableSamplers = nullptr;
            bindings.push_back(_binding);

            VkDescriptorPoolSize _size = {};
            _size.type = _descriptor_type;
            _size.descriptorCount = _array_size;
            sizes.push_back(_size);
        }break;


        default:
        {
            SmoothieCore::logError_Formated(R"({} Unknown or unsupported format type: {})", SMOOTHIE_LOCATION, static_cast<int>(_type.shape));
            return 1;
        }break;
    }

    return 0;
}

static int get_struct_info(
    const Shader_Type_Struct& type_struct,
    std::vector<VkDescriptorSetLayoutBinding> &bindings,
    std::vector<VkDescriptorPoolSize> &sizes,
    unsigned int& _set_binding_offset,
    const std::unordered_map<std::string, Shader_Type_Base_ptr>& _all_types, VkShaderStageFlags stages)
{
    for (const auto& _member: type_struct.members)
    {
        if (const auto& _type = _all_types.find(_member.type); _type == _all_types.end())
        {
            //std::cout << "Could not find type \"" << _member.type << "\" of a variable named \"" << _member.name << "\"" << std::endl;
            return 1;
        }

        const auto& _type = _all_types.at(_member.type);
        if (_type == nullptr)
        {
            //std::cout << "Type: " << _member.name << " does not have a reflection!" << std::endl;
            return 1;
        }

        switch (_type->kind)
        {
            default:
            {
                //std::cout << "Unknown Kind of reflected object for member: " << _member.name << std::endl;
                return 1;
            }break;

            case Shader_Type_Kind::Uniform:
            {

                const auto& _uniform = std::dynamic_pointer_cast<Shader_Type_Uniform>(_type);
                if (_uniform == nullptr)
                {
                    //std::cout << R"(Failed to cast type from "Shader_Type_Base" to "Shader_Type_Resource" for variable ")" << _member.name << std::endl;
                    return 1;
                }

                if (_all_types.find(_uniform->userType) == _all_types.end())
                {
                    continue;
                    // std::cout << "No type named: " << _uniform->userType << std::endl;
                    // return 1;
                }

                const auto& _user_type = _all_types.at(_uniform->userType);
                if (_user_type == nullptr)
                {
                   // std::cout << R"(Failed to cast type from "Shader_Type_Base" to "Shader_Type_Uniform" for variable ")" << _member.name << std::endl;
                }


                if (std::dynamic_pointer_cast<Shader_Type_Struct>(_user_type) != nullptr)
                {
                   // std::cout << "Feature not yet implemented! Can't do array for structs in this context! " << __FILE__ << ":" << std::to_string(__LINE__) << std::endl;
                    return 1;
                }

                const auto& _user_type_rss = std::dynamic_pointer_cast<Shader_Type_Resource>(_user_type);
                if (_user_type_rss == nullptr){continue;}

                if (get_resource_descriptor_data(*_user_type_rss, bindings, sizes, _set_binding_offset, stages, _uniform->count) != 0)
                {
                   // std::cout << "Unable to parse element: " << _member.name << std::endl;
                    return 1;
                }
                _set_binding_offset += 1;

            }break;

            case Shader_Type_Kind::Resource:
            {
                const auto& _resource = std::dynamic_pointer_cast<Shader_Type_Resource>(_type);
                if (_resource == nullptr)
                {
                    //std::cout << R"(Failed to cast type from "Shader_Type_Base" to "Shader_Type_Resource" for variable ")" << _member.name << std::endl;
                    return 1;
                }

                if (get_resource_descriptor_data(*_resource, bindings, sizes, _set_binding_offset, stages) != 0)
                {
                    //std::cout << "Unable to parse element: " << _member.name << std::endl;
                    return 1;
                }
                _set_binding_offset += 1;

            }break;

            case Shader_Type_Kind::Struct:
            {
                const auto& _struct = std::dynamic_pointer_cast<Shader_Type_Struct>(_type);
                if (_struct == nullptr)
                {
                    //std::cout << R"(Failed to cast type from "Shader_Type_Base" to "Shader_Type_Struct" for variable ")" << _member.name<< "\"!"  << std::endl;
                    return 1;
                }

                if (get_struct_info(*_struct, bindings, sizes, _set_binding_offset, _all_types, stages) != 0)
                {
                    //std::cout << "Failed to get info from struct." << std::endl;
                    return 1;
                }

            } break;
        }


    }

    return 0;
}


int ShaderFile::get_DescriptorData(
    const std::string &globalVarName,
    std::vector<VkDescriptorSetLayoutBinding> &bindings,
    std::vector<VkDescriptorPoolSize> &sizes,
    VkShaderStageFlags stages) const
{
    Variable_Global _globalDescriptorSet;
    for (const auto& _global_variable : m_GlobalVariables)
    {
        if (_global_variable.name == globalVarName)
        {
            _globalDescriptorSet = _global_variable;
            break;
        }
    }

    if (_globalDescriptorSet.name.empty())
    {
        //std::cout << "Failed to get global descriptor set variable!" << std::endl;
        return 1;
    }


    if (m_Types.empty())
    {
        //std::cout << "No types in the shader!" << std::endl;
        return 1;
    }

    if (const auto& _struct = m_Types.find(_globalDescriptorSet.type); _struct == m_Types.end())
    {
        //std::cout << "Failed to get global descriptor set type!" << std::endl;
        return 1;
    }

    const auto& _global_type = m_Types.at(_globalDescriptorSet.type);
    if (_global_type == nullptr)
    {
       // std::cout << "Failed to get global descriptor set type!" << std::endl;
        return 1;
    }

    if (_global_type->kind != Shader_Type_Kind::Struct)
    {
        //std::cout << "Global descriptor type set MUST be struct!" << std::endl;
        return 1;
    }

    const auto _global_type_struct = std::dynamic_pointer_cast<Shader_Type_Struct>(_global_type);
    if (_global_type_struct == nullptr)
    {
        //std::cout << "Failed to cast the type from Shader_Type_Base to Shader_Type_Struct" << std::endl;
        return 1;
    }

    unsigned int _binding_offset = 0;
    for (const auto& _member: _global_type_struct->members)
    {
        if (_member.uniform_offset != 0)
        {

            VkDescriptorSetLayoutBinding _binding = {};
            _binding.binding = 0;
            _binding.descriptorCount = 1;
            _binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            _binding.pImmutableSamplers = nullptr;
            bindings.push_back(_binding);
            sizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1});
            _binding_offset = 1;
        }
    }


    if (get_struct_info(*_global_type_struct, bindings, sizes, _binding_offset, m_Types, stages))
    {
        //std::cout << "Failed to get info from struct." << std::endl;
        return 1;
    }


    return 0;
}

int ShaderFile::create(const std::string& shaderFile)
{
	m_Filepath = shaderFile;
	if (!std::filesystem::exists(m_Filepath))
	{
	    SmoothieCore::logError_Formated("No file named: {}", m_Filepath);
		return 1;
	}

	const auto file_size = std::filesystem::file_size(m_Filepath);
	std::vector<char> binary_data(file_size);
	auto file = std::ifstream(m_Filepath, std::ios_base::binary);

	if (de_serialize(file) != 0)
	{
	    SmoothieCore::logError_Formated("Failed to parse file: {}", m_Filepath);
		file.close();
		return 1;
	}
	file.close();

	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = m_SPIR_V_CODE.size() * sizeof(uint32_t);
	assert(shaderModuleCreateInfo.codeSize != 0);
	shaderModuleCreateInfo.pCode = m_SPIR_V_CODE.data();
	assert(shaderModuleCreateInfo.pCode != nullptr);
	shaderModuleCreateInfo.pNext = nullptr;
	shaderModuleCreateInfo.flags = 0;

	if (
		!static_cast<int>(m_Flags ^ ShaderFile_CreateFlags::DontCreateModule) &&
		vkCreateShaderModule(SmoothieCore::getDevice(), &shaderModuleCreateInfo, nullptr, &m_ShaderModule) != VK_SUCCESS)
	{
	    SmoothieCore::logError_Formated("{} Failed to create shader module!", SMOOTHIE_LOCATION);
		return 1;
	}
    
	if (!static_cast<int>(m_Flags ^ ShaderFile_CreateFlags::Hold_SPIR_V))
	{
		m_SPIR_V_CODE.clear();
	}

    SmoothieCore::logVerbose_Formated("Created ShaderFile: {}", m_Filepath);
	return 0;
}

void ShaderFile::destroy()
{
	if (m_ShaderModule != nullptr)
	{
		vkDestroyShaderModule(SmoothieCore::getDevice(), m_ShaderModule, nullptr);
		m_ShaderModule = nullptr;
	}
    SmoothieCore::logVerbose_Formated("Destroyed ShaderFile: {}", m_Filepath);
}

