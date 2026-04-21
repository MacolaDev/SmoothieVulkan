#include "../Smoothie.slang"

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


[vk_binding(0, 1)]
cbuffer Data
{
    RWStructuredBuffer<pc_out> Out;
    Sampler2D depthImage;
    Texture2D<uint> ModelIDImage;
    Texture2D<uint> InstanceIDImage;
}


[shader("compute")]
[numthreads(1,1,1)]
void compute_GPU_SELECTED_DATA()
{
    float s_depth = depthImage.Sample(uv).r;

    uint2 size;
    ModelIDImage.GetDimensions(size.x, size.y);

    uint2 texel = uint2(uv * float2(size));
    texel = clamp(texel, uint2(0), size - 1);
    
    Out[0].ModelID = ModelIDImage.Load(int3(texel, 0)).r;

    Out[0].InstanceID = InstanceIDImage.Load(int3(texel, 0)).r;

    float4 s_world_pos = mul(Camera.invProjectionViewMatrix, float4(float2(uv) * 2.0f - 1.0f, s_depth, 1.0f));
    s_world_pos.xyzw /= s_world_pos.w;

    Out[0].worldPos = s_world_pos.xyz;
    Out[0].depth = s_depth;
}

