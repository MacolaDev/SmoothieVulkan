#include "../include/Smoothie.slang"

struct InputData
{
    Sampler2D gDepth;
    Sampler2D gNormal;
    Sampler2D texNoise;
    Buffer<float4, 64> samples;
}
ParameterBlock<Smoothie_Globals> g_SmoothieGlobals;
ParameterBlock<InputData> g_InputData;

static const float radius = 0.4f;
static const int kernelSize = 16;

[shader("fragment")]
float4 fragment_SSAO(float2 TexCoords)
{
    var _camera = g_SmoothieGlobals.camera;

    float4 FragPos = mul(_camera.invProjectionViewMatrix, float4(TexCoords * 2.0f - 1.0f, g_InputData.gDepth.Sample(TexCoords).x, 1.0f)); //FragPos is in camera space
	FragPos.xyzw /= FragPos.w;

    float3 normal = normalize(g_InputData.gNormal.Sample(TexCoords).xyz); //normal is in camera space

    // float2 noiseScale = float2(_camera.SCR_WIDTH / 4.0f, _camera.SCR_HEIGHT / 4.0f);
    float2 noiseScale = float2(0.5f, 0.5f);
    float3 randomVec = g_InputData.texNoise.Sample(TexCoords * noiseScale).xyz;
	
	float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	float3 bitangent = cross(normal, tangent);
	float3x3 TBN = float3x3(tangent, bitangent, normal);

	
	float occlusion = 0.0f;
	for(int i = 0; i < kernelSize; i++)
	{
        // get sample position
        float3 SAMPLE = mul(TBN, g_InputData.samples[i].xyz); // from tangent to camera space
		SAMPLE = FragPos.xyz + SAMPLE * radius;

        float4 offset = float4(SAMPLE, 1.0f);
        offset = mul(_camera.projectionViewMatrix, offset); // from camera to clip-space
		offset.xyzw /= offset.w;
		offset.xy = offset.xy * 0.5f + 0.5f;

        float sampleDepth = g_InputData.gDepth.Sample(offset.xy).x;
		
		float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(offset.z - sampleDepth));
		occlusion += (sampleDepth >= offset.z ? 1.0f : 0.0f) * rangeCheck;
	}

	occlusion =  (occlusion / kernelSize);
	return float4(float3(occlusion), 1.0f);
}
