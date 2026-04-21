#pragma once

// Disables reflecting a struct or a parameter.
[__AttributeUsage(_AttributeTargets.Struct)]
struct NoReflectionAttribute{}

//Mandatory for all shaders to be set.
//With this we identify shader inside a file.
[__AttributeUsage(_AttributeTargets.Function)]
struct TargetPipelineAttribute
{
    string pipeline;
};

[__AttributeUsage(_AttributeTargets.Var)]
struct VkSetAttribute
{
    int set;   
}

#define VK_SET(n) [vk_binding(0,n)] [VkSet(n)]

[NoReflection]
struct CameraData
{
    float4x4 projectionMatrix;
    float4x4 invProjectionMatrix;
    float4x4 viewMatrix;
    float3 viewPos;
    float4x4 projectionViewMatrix;
    float4x4 invProjectionViewMatrix;
    uint SCR_WIDTH;
    uint SCR_HEIGHT;
};

[VkSet(0)]
ConstantBuffer<CameraData> Camera;

[NoReflection]
struct EngineParams
{
}

[VkSet(1)]
ConstantBuffer<EngineParams> g_EngineParams;


struct gBufferPass
{
    float4 gPosition;
    float4 gNormal;
    float4 gAlbedo;
    float4 gMRAO;
}

struct HDRPass
{
    float4 HDR; 
}


float4 unpackNormals(uint packedNormal)
{
    float x = ((packedNormal >> 0) & 0xFF);
    float y = ((packedNormal >> 8) & 0xFF);
    float z = ((packedNormal >> 16) & 0xFF);
    float w = ((packedNormal >> 24) & 0xFF);

    float4 result = float4(x, y, z, w);

    result /= float4(127);
    result -= float4(1);

    return result;
}