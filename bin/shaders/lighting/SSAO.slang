#include "../Smoothie.slang"

[vk_binding(0, 2)]
cbuffer InputData
{
    Sampler2D gDepth;
    Sampler2D gNormal;
    Sampler2D texNoise;
    Buffer<float4, 64> samples;
}

static const float radius = 0.4f;
static const int kernelSize = 16;

[shader("fragment")]
float4 fragment_SSAO(float2 TexCoords)
{
    float4 FragPos = mul(Camera.invProjectionViewMatrix, float4(TexCoords * 2.0f - 1.0f, gDepth.Sample(TexCoords).x, 1.0f)); //FragPos is in camera space
	FragPos.xyzw /= FragPos.w;

    float3 normal = normalize(gNormal.Sample(TexCoords).xyz); //normal is in camera space

    float2 noiseScale = float2(Camera.SCR_WIDTH / 4.0f, Camera.SCR_HEIGHT / 4.0f);
    float3 randomVec = texNoise.Sample(TexCoords * noiseScale).xyz;
	
	float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	float3 bitangent = cross(normal, tangent);
	float3x3 TBN = float3x3(tangent, bitangent, normal);

	
	float occlusion = 0.0f;
	for(int i = 0; i < kernelSize; i++)
	{	
		//get sample position
		float3 SAMPLE = mul(TBN, samples[i].xyz); // from tangent to camera space
		SAMPLE = FragPos.xyz + SAMPLE * radius;

        float4 offset = float4(SAMPLE, 1.0f);
        offset = mul(Camera.projectionViewMatrix, offset); // from camera to clip-space
		offset.xyzw /= offset.w;
		offset.xy = offset.xy * 0.5f + 0.5f;

        float sampleDepth = gDepth.Sample(offset.xy).x;
		
		float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(offset.z - sampleDepth));
		occlusion += (sampleDepth >= offset.z ? 1.0f : 0.0f) * rangeCheck;
	}

	occlusion =  (occlusion / kernelSize);
	return float4(float3(occlusion), 1.0f);
}
