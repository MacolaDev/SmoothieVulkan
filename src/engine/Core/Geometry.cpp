//
// Created by macola on 6/27/26.
//

#include "Geometry.h"
#include <fstream>

#include "SmoothieCore.h"

using namespace Smoothie;

static void read_string(std::ifstream& file, std::string& str)
{
    unsigned int _size = 0;
    file.read(reinterpret_cast<std::istream::char_type*>(&_size), sizeof(_size));
    str.resize(_size);
    file.read(reinterpret_cast<char*>(str.data()), _size);
}

static void read_type(std::ifstream& file, Geometry_Data_Type& type)
{
    file.read(reinterpret_cast<std::istream::char_type *>(&type.base_type), sizeof(type.base_type));
    file.read(reinterpret_cast<std::istream::char_type *>(&type.count), sizeof(type.count));
}

static void read_buffer(std::ifstream& file, std::vector<std::uint8_t>& buffer)
{
    unsigned int _size = 0;
    file.read(reinterpret_cast<std::istream::char_type *>(&_size), sizeof(_size));
    buffer.resize(_size);
    file.read(reinterpret_cast<std::istream::char_type *>(buffer.data()), _size);
}

static void read_mesh(std::ifstream& file, Geometry_Mesh& mesh)
{
    file.read(reinterpret_cast<std::istream::char_type *>(&mesh.index_type), sizeof(mesh.index_type));
    file.read(reinterpret_cast<std::istream::char_type *>(&mesh.index_topology), sizeof(mesh.index_topology));

    unsigned int _vertex_data_size = 0;
    file.read(reinterpret_cast<std::istream::char_type *>(&_vertex_data_size), sizeof(_vertex_data_size));
    mesh.vertex_buffer_attributes.resize(_vertex_data_size);
    for (unsigned int i = 0; i < _vertex_data_size; i++)
    {
        auto& _attribute = mesh.vertex_buffer_attributes[i];

        file.read(reinterpret_cast<std::istream::char_type *>(&_attribute.offset), sizeof(_attribute.offset));
        file.read(reinterpret_cast<std::istream::char_type *>(&_attribute.size), sizeof(_attribute.size));

        read_type(file, _attribute.type);
        read_type(file, _attribute.original_type);

        read_buffer(file, _attribute.min_values);
        read_buffer(file, _attribute.max_values);
    }

    unsigned int _index_ranges = 0;
    file.read(reinterpret_cast<std::istream::char_type *>(&_index_ranges), sizeof(_index_ranges));
    mesh.LODs.resize(_index_ranges);
    for (unsigned int i = 0; i < _index_ranges; i++)
    {
        auto& _lod = mesh.LODs[i];
        file.read(reinterpret_cast<std::istream::char_type *>(&_lod.vertex_offset), sizeof(_lod.vertex_offset));
        file.read(reinterpret_cast<std::istream::char_type *>(&_lod.vertex_count), sizeof(_lod.vertex_count));
        file.read(reinterpret_cast<std::istream::char_type *>(&_lod.index_offset), sizeof(_lod.index_offset));
        file.read(reinterpret_cast<std::istream::char_type *>(&_lod.index_count), sizeof(_lod.index_count));
    }

    read_buffer(file, mesh.vertex_data_raw);
    read_buffer(file, mesh.index_data_raw);
}

static void read_meshes(std::ifstream& file, GeometryFile::MeshMap& meshes)
{
    unsigned int _n_of_meshes = 0;
    file.read(reinterpret_cast<std::istream::char_type *>(&_n_of_meshes), sizeof(unsigned int));
    for (unsigned int i = 0; i < _n_of_meshes; i++)
    {
        std::string _name;
        read_string(file, _name);

        Geometry_Mesh _mesh;
        read_mesh(file, _mesh);

        meshes.insert({_name, std::move(_mesh)});
    }

}

int Smoothie::GeometryFile::create(const std::string& filepath, GeometryFile_CreateFlags flags)
{
    if (!std::filesystem::exists(filepath))
    {
        SmoothieCore::logWarning("No file named: "+ filepath);
        return 1;
    }

    std::ifstream file(filepath, std::ios::in);

    char _magic[4] = {};
    file.read(_magic, sizeof(_magic));
    if (std::strncmp(_magic, s_Magic, sizeof(_magic)) != 0)
    {
        SmoothieCore::logError("Magic number for " + filepath + " does not match with expected one");
        return 1;
    }

    try
    {
        read_meshes(file, m_Meshes);
    } catch (std::exception& e)
    {
        SmoothieCore::logError("Failed to read: " + filepath + "; with error: " + e.what());
    }

    file.close();


    return create_buffers(m_Meshes, flags);
}

void Smoothie::GeometryFile::destroy()
{
    for (auto& [_name, _mesh] : m_Meshes)
    {
        if (_mesh.vertex_buffer_allocation != nullptr)
        {
            vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), _mesh.vertex_buffer, _mesh.vertex_buffer_allocation);
            _mesh.vertex_buffer = nullptr;
            _mesh.vertex_buffer_allocation = nullptr;
        }

        if (_mesh.index_buffer_allocation != nullptr)
        {
            vmaDestroyBuffer(SmoothieCore::getVulkanMemoryAllocator(), _mesh.index_buffer, _mesh.index_buffer_allocation);
            _mesh.index_buffer_allocation = nullptr;
            _mesh.index_buffer_allocation = nullptr;
        }

    }
}

int GeometryFile::create_buffers(MeshMap &meshes, GeometryFile_CreateFlags flags)
{
    VmaAllocationCreateInfo _allocation_info = {};
    _allocation_info.usage = VMA_MEMORY_USAGE_AUTO;
    _allocation_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    for (auto& [_name, mesh]: m_Meshes)
    {
        VkBufferCreateInfo vertex_buffer_create_info = {};
        vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vertex_buffer_create_info.pNext = nullptr;
        vertex_buffer_create_info.flags = 0;
        vertex_buffer_create_info.size = mesh.vertex_data_raw.size();
        vertex_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vertex_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vertex_buffer_create_info.queueFamilyIndexCount = 0;
        vertex_buffer_create_info.pQueueFamilyIndices = nullptr;
        if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(),
            &vertex_buffer_create_info,
            &_allocation_info,
            &mesh.vertex_buffer,
            &mesh.vertex_buffer_allocation, nullptr) != VK_SUCCESS)
        {
            SmoothieCore::logError("Failed to create vertex buffer for mesh: " + _name);
            destroy();
            return 1;
        }

        if (vmaCopyMemoryToAllocation(SmoothieCore::getVulkanMemoryAllocator(),
            mesh.vertex_data_raw.data(),
            mesh.vertex_buffer_allocation, 0,
            mesh.vertex_data_raw.size()) != VK_SUCCESS)
        {
            SmoothieCore::logError("Failed to copy raw vertex data to GPU for mesh: " + _name);
            destroy();
            return 1;
        }

        VkBufferCreateInfo index_buffer_create_info = vertex_buffer_create_info;
        index_buffer_create_info.size = mesh.index_data_raw.size();
        index_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (vmaCreateBuffer(SmoothieCore::getVulkanMemoryAllocator(),
            &index_buffer_create_info,
            &_allocation_info,
            &mesh.index_buffer,
            &mesh.index_buffer_allocation, nullptr) != VK_SUCCESS)
        {
            SmoothieCore::logError("Failed to create index buffer for mesh: " + _name);
            destroy();
            return 1;
        }

        if (vmaCopyMemoryToAllocation(SmoothieCore::getVulkanMemoryAllocator(),
            mesh.index_data_raw.data(),
            mesh.index_buffer_allocation, 0,
            mesh.index_data_raw.size()) != VK_SUCCESS)
        {
            SmoothieCore::logError("Failed to copy raw index data to GPU for mesh: " + _name);
            destroy();
            return 1;
        }

    }


    return 0;
}
