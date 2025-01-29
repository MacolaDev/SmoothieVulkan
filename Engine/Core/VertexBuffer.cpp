#include "VertexBuffer.h"

std::vector<VkVertexInputAttributeDescription> xyznuvtb::getVertexAttributes() const
{
    std::vector<VkVertexInputAttributeDescription> descriptors;
    
    //Position 
    VkVertexInputAttributeDescription position{};
    position.binding = 0;
    position.location = 0;
    position.format = VK_FORMAT_R32G32B32_SFLOAT;
    position.offset = 0;
    descriptors.push_back(position);

    //Normals
    VkVertexInputAttributeDescription normal{};
    normal.binding = 0;
    normal.location = 1;
    normal.format = VK_FORMAT_R32_UINT;
    normal.offset = 3 * sizeof(float);
    descriptors.push_back(normal);

    //UV coordinates
    VkVertexInputAttributeDescription uv{};
    uv.binding = 0;
    uv.location = 2;
    uv.format = VK_FORMAT_R32G32_SFLOAT;
    uv.offset = 3 * sizeof(float) + sizeof(unsigned int);
    descriptors.push_back(uv);

    //Tangents
    VkVertexInputAttributeDescription tangent{};
    tangent.binding = 0;
    tangent.location = 3;
    tangent.format = VK_FORMAT_R32_UINT;
    tangent.offset = 3 * sizeof(float) + sizeof(unsigned int) + 2 * sizeof(float);
    descriptors.push_back(tangent);

    //Bitangents
    VkVertexInputAttributeDescription bitangent{};
    bitangent.binding = 0;
    bitangent.location = 4;
    bitangent.format = VK_FORMAT_R32_UINT;
    bitangent.offset = 3 * sizeof(float) + sizeof(unsigned int) + 2 * sizeof(float) + sizeof(unsigned int);
    descriptors.push_back(bitangent);
    
    return descriptors;
}

VkVertexInputBindingDescription xyznuvtb::getBindingDescription() const
{
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = 5 * sizeof(float) + 3 * sizeof(unsigned int);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> xyznuvtbc::getVertexAttributes() const
{
    return std::vector<VkVertexInputAttributeDescription>();
}

VkVertexInputBindingDescription xyznuvtbc::getBindingDescription() const
{
    return VkVertexInputBindingDescription();
}



std::vector<VkVertexInputAttributeDescription> VertexBufferBase::getVertexAttributes() const
{
    return std::vector<VkVertexInputAttributeDescription>();
}
VkVertexInputBindingDescription VertexBufferBase::getBindingDescription() const
{
    return VkVertexInputBindingDescription();
}
