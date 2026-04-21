
[vk_push_constant]
cbuffer pc
{
    float4x4 swizzlerMatrix;
};

[vk_binding(0, 0)]
cbuffer Data
{
    Sampler2D srcImage;
    RWTexture2D<float4> dstImage;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_SWIZZLER(uint3 invocationID: SV_GroupID)
{

    // Get UV coordinates
    int2 coord = int2(invocationID.xy);
    int2 size;
    dstImage.GetDimensions(size.x, size.y);
    float2 uv = float2(coord) / float2(size);
    uv.y = 1.0f - uv.y;

    float4 source = srcImage.Sample(uv).rgba;
    float4 result = mul(swizzlerMatrix, source);
    result.xyz = clamp(result.xyz, float3(0.0f), float3(1.0f));
    dstImage.Store(coord, float4(result.xyz, 1.0f));
}
