#include "../include/Smoothie.slang"

struct Smoothie_VerexInput
{
    float3 aPos;
    float3 aNormals;
    float2 aTexCoord;
    float3 aTangents;
}

struct VS_Output
{
    float4 pos;
    float2 TexCoord;
    float4 FragPos;
    float3x3 TBN;
}

struct ModelData
{
    float4x4 modelMatrix;
    float4x4 normalMatrix;
    uint ModelID;

    Sampler2D diffuse;
    Sampler2D normalMap;
    Sampler2D metalnessMap;
    Sampler2D roughnessMap;
}
ParameterBlock<Smoothie_Globals> g_SmoothieGlobals;
ParameterBlock<ModelData> g_ModelData;

[TargetPipeline("standard")]
[shader("vertex")]
VS_Output mainVertex(Smoothie_VerexInput in, out float4 position: SV_Position)
{

    var _camera = g_SmoothieGlobals.camera;
    var _model_matrix = g_ModelData.modelMatrix;

    VS_Output out;
    out.TexCoord = in.aTexCoord;
    out.FragPos = mul(_model_matrix, float4(in.aPos, 1.0f));

    float3 T = normalize(mul(_model_matrix, float4(in.aTangents.xyz, 0.0f))).xyz;
    float3 N = normalize(mul(_model_matrix, float4(in.aNormals.xyz, 0.0f))).xyz;
	T = normalize(T - dot(T, N) * N);
	float3 B = cross(N, T);
    out.TBN = float3x3(T, B, N);
    position = out.pos = mul(_camera.projectionViewMatrix, out.FragPos);
    return out;
}

[TargetPipeline("standard")]
[shader("fragment")]
FS_Output_gBuffer mainFragment(VS_Output in)
{
    FS_Output_gBuffer out;
    out.mrao = float4(0.0f, 0.0f, 1.0f, 1.0f);
    out.mrao.r = g_ModelData.metalnessMap.Sample(in.TexCoord).r;
    out.mrao.g = g_ModelData.roughnessMap.Sample(in.TexCoord).r;
    out.mrao.b = 1.0f;

    out.position = in.FragPos;
    out.albedo.rgba = g_ModelData.diffuse.Sample(in.TexCoord).rgba;

    float3 normal = g_ModelData.normalMap.Sample(in.TexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    out.normal = float4(normalize(mul(in.TBN, normal)), 1.0f);
    return out;
}
