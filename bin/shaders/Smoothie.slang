#pragma once
#include "Attributes.slang"

struct Attachments
{
    Texture2D gPosition;
    Texture2D gNormal;
    Texture2D gAlbedo;
    Texture2D gMRAO;

    Texture2D HDR;

    Texture2D depth;
}


struct Samplers
{
    SamplerState Texture2DSampler;
    SamplerState ClampToEdgeLINEAR;
    SamplerState ClampToEdgeNEAREST;
    SamplerState RepeatNEAREST;
}

struct Camera
{
    float4x4 projectionMatrix;
    float4x4 invProjectionMatrix;
    float4x4 viewMatrix;
    float3 viewPos;
    float4x4 projectionViewMatrix;
    float4x4 invProjectionViewMatrix;
}

struct ShadowCamera
{
    float4x4 projectionViewMatrix;
}

struct Smoothie_Globals
{
    Attachments attachments;
    Samplers samplers;
    ConstantBuffer<Camera> camera;
    ConstantBuffer<ShadowCamera> shadowCamera;
}

struct FS_Output_gBuffer
{
    float4 position;
    float4 normal;
    float4 albedo;
    float4 mrao;
}


struct FS_Output_HDR
{
    float4 hdr;
}