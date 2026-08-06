#include "../include/Smoothie.slang"

[vk_push_constant]
cbuffer pc_in
{
    float2 uv;
}

struct pc_out
{
    uint ModelID;
    uint InstanceID;
    float depth;
    float3 worldPos;
}

struct InputData
{
    RWStructuredBuffer<pc_out> Out;
    Sampler2D depthImage;
    Texture2D<uint> ModelIDImage;
    Texture2D<uint> InstanceIDImage;
}

ParameterBlock<Smoothie_Globals> g_SmoothieGlobals;
ParameterBlock<InputData> g_InputData;


[shader("compute")]
[numthreads(1,1,1)]
void compute_GPU_SELECTED_DATA()
{
    var _camera = g_SmoothieGlobals.camera;

    float s_depth = g_InputData.depthImage.Sample(uv).r;

    uint2 size;
    g_InputData.ModelIDImage.GetDimensions(size.x, size.y);
    
    uint2 texel = uint2(uv * float2(size));
    texel = clamp(texel, uint2(0), size - 1);

    g_InputData.Out[0].ModelID = g_InputData.ModelIDImage.Load(int3(texel, 0)).r;

    g_InputData.Out[0].InstanceID = g_InputData.InstanceIDImage.Load(int3(texel, 0)).r;

    float4 s_world_pos = mul(_camera.invProjectionViewMatrix, float4(float2(uv) * 2.0f - 1.0f, s_depth, 1.0f));
    s_world_pos.xyzw /= s_world_pos.w;

    g_InputData.Out[0].worldPos = s_world_pos.xyz;
    g_InputData.Out[0].depth = s_depth;
}

