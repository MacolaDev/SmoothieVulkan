#include "../Smoothie.slang"

[vk_binding(0, 2)]
cbuffer ModelData
{
    float4x4 modelMatrix;
    float4x4 normalMatrix;
    uint ModelID;

    Sampler2D diffuse;
    Sampler2D normalMap;
    Sampler2D metalnessMap;
    Sampler2D roughnessMap;
}

struct Smoothie_VerexInput
{
    float3 aPos;
    uint aNormals;
    float2 aTexCoord;
    uint aTangents;
    uint aBitangents;
}

struct VS_Output
{
    float4 pos;
    float2 TexCoord;
    float4 FragPos;
    float3x3 TBN;
}

[TargetPipeline("standard")]
[shader("vertex")]
VS_Output mainVertex(Smoothie_VerexInput in, int magic, out float4 position: SV_Position)
{
    VS_Output out;
    out.TexCoord = in.aTexCoord;
    out.FragPos = mul(modelMatrix, float4(in.aPos, 1.0f));
    
	float3 T = normalize(mul(modelMatrix, float4(unpackNormals(in.aTangents).xyz, 0.0f))).xyz;
	float3 N = normalize(mul(modelMatrix, float4(unpackNormals(in.aNormals).xyz, 0.0f))).xyz;
	T = normalize(T - dot(T, N) * N);
	float3 B = cross(N, T);
    out.TBN = float3x3(T, B, N);
    position = out.pos = mul(Camera.projectionViewMatrix, out.FragPos);
    return out;
}

[TargetPipeline("standard")]
[shader("fragment")]
gBufferPass mainFragment(VS_Output in)
{
    gBufferPass out;
    out.gMRAO = float4(0.0f, 0.0f, 1.0f, 1.0f);
    out.gMRAO.r = metalnessMap.Sample(in.TexCoord).r;
    out.gMRAO.g = roughnessMap.Sample(in.TexCoord).r;
    out.gMRAO.b = 1.0f;
    
	out.gPosition = in.FragPos;
    out.gAlbedo.rgba = diffuse.Sample(in.TexCoord).rgba;
	
	float3 normal = normalMap.Sample(in.TexCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	out.gNormal = float4(normalize(mul(in.TBN, normal)), 1.0f);
    return out;
}
