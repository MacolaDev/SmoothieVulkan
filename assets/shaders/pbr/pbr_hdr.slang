#include "../include/Smoothie.slang"
#include "../include/helpers.slang"

struct VertexShaderInput
{
    float3 aPos;
    uint aNormals;
    float2 aTexCoord;
    uint aTangents;
    uint aBitangents;
}

struct VertexShaderOutput
{
    float2 TexCoord;
    float4 FragPos;
    float3x3 TBN;
}

struct ModelData
{
	float4 modelMatrix;
	float4 normalMatrix;
	uint ModelID;

	Sampler2D diffuse;
	Sampler2D normalMap;
	Sampler2D metalnessMap;
	Sampler2D roughnessMap;
};

ParameterBlock<Smoothie_Globals> g_SmoothieGlobals;
ParameterBlock<ModelData> g_ModelData;

[shader("vertex")]
[TargetPipeline("standard")]
VertexShaderOutput mainVertex(VertexShaderInput in, out float4 position: SV_Position)
{
    VertexShaderOutput out;
    out.TexCoord = in.aTexCoord;
    out.FragPos = mul(g_ModelData.modelMatrix, float4(in.aPos, 1.0f));

    float3 T = normalize(mul(g_ModelData.modelMatrix, unpackNormals(in.aTangents)));
    float3 N = normalize(mul(g_ModelData.modelMatrix, unpackNormals(in.aNormals)));
	T = normalize(T - dot(T, N) * N);
	float3 B = cross(N, T);
    out.TBN = float3x3(T, B, N);
    position = mul(g_SmoothieGlobals.camera.projectionViewMatrix, out.FragPos);
    return out; 
}

[shader("fragment")]
[TargetPipeline("standard")]
float4 mainFragment(VertexShaderOutput in)
{
	// gMRAO = vec4(0.0f, 0.0f, 1.0f, 1.0f);

	// gMRAO.r = texture(metalnessMap, TexCoord).r;
	// gMRAO.g = texture(roughnessMap, TexCoord).r;
	// gMRAO.b = 1.0f;

	return float4(120.0f, 10.0f, 90.0f, 1.0f);

	
	// vec3 normal = texture(normalMap, TexCoord).rgb;
	// normal = normalize(normal * 2.0 - 1.0);
	// gNormal = vec4(normalize(TBN * normal), 1);
	
	// gAlbedoSpec.rgba = texture(diffuse, TexCoord).rgba;
	
}
