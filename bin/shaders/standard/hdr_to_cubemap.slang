
static const float2 invAtan = float2(0.1591, 0.3183);

float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

[vk_binding(0, 0)]
Sampler2D skybox;

[shader("fragment")]
float4 fragment_HDR_TO_CUBEMAP(float3 TexCoords)
{
    float2 uv = SampleSphericalMap(normalize(TexCoords));
    float3 color = skybox.Sample(uv).rgb;
    return float4(color, 1.0);
}