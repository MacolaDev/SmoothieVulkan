#include "VertexBuffer.h"
#include "Math/SmoothieMath.h"
using namespace SmoothieMath;
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

void xyznuvtb::populateBindingDescription(VkVertexInputBindingDescription& bindingDescription) const
{
    bindingDescription.binding = 0;
    bindingDescription.stride = 5 * sizeof(float) + 3 * sizeof(unsigned int);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

std::vector<VkVertexInputAttributeDescription> xyznuvtbc::getVertexAttributes() const
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
    normal.offset = sizeof(Vector3); //Position
    descriptors.push_back(normal);

    //UV coordinates
    VkVertexInputAttributeDescription uv{};
    uv.binding = 0;
    uv.location = 2;
    uv.format = VK_FORMAT_R32G32_SFLOAT;
    uv.offset = 
        3 * sizeof(float) //Position
        + sizeof(unsigned int); //Normal

    descriptors.push_back(uv);

    //Tangents
    VkVertexInputAttributeDescription tangent{};
    tangent.binding = 0;
    tangent.location = 3;
    tangent.format = VK_FORMAT_R32_UINT;
    tangent.offset = 
        3 * sizeof(float)  //position
        + sizeof(unsigned int) //normal
        + 2 * sizeof(float); //uv
    descriptors.push_back(tangent);

    //Bitangents
    VkVertexInputAttributeDescription bitangent{};
    bitangent.binding = 0;
    bitangent.location = 4;
    bitangent.format = VK_FORMAT_R32_UINT;
    bitangent.offset = 
        3 * sizeof(float) //position
        + sizeof(unsigned int) //normal
        + 2 * sizeof(float) //uv
        + sizeof(unsigned int); //tangents
    descriptors.push_back(bitangent);

    //Color 
    VkVertexInputAttributeDescription color{};
    color.binding = 0;
    color.location = 5;
    color.format = VK_FORMAT_R32_UINT;
    color.offset =
        3 * sizeof(float) //position
        + sizeof(unsigned int) //normal
        + 2 * sizeof(float) //uv
        + sizeof(unsigned int) //t
        + sizeof(unsigned int);//b

    descriptors.push_back(color);
    return descriptors;
}

void xyznuvtbc::populateBindingDescription(VkVertexInputBindingDescription& bindingDescription) const
{
    bindingDescription.binding = 0;
    bindingDescription.stride = 5 * sizeof(float) + 4 * sizeof(unsigned int);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}


std::vector<VkVertexInputAttributeDescription> VertexBufferBase::getVertexAttributes() const
{
    return std::vector<VkVertexInputAttributeDescription>();
}
void VertexBufferBase::populateBindingDescription(VkVertexInputBindingDescription& bindingDescription) const
{
}

