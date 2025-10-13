#version 450
#extension GL_ARB_shading_language_include: enable 
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable
#include "../ShadersGeneral.glslh"

#define DESCRIPTOR_CAMERA MAKE_DESCRIPTOR_SET(0, 0)
#define DESCRIPTOR_GENERAL MAKE_DESCRIPTOR_SET(1, 0)
#include "../Descriptors.glslh"


//************* SSAO *************//
#ifdef SSHADER_FRAGMENT_SSAO
layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 FragColor;

layout (set = 2, binding = 0) uniform sampler2D gDepth;
layout (set = 2, binding = 1) uniform sampler2D gNormal;
layout (set = 2, binding = 2) uniform sampler2D texNoise;
layout (set = 2, binding = 3) uniform SAMPLES
{
	vec4 samples[64];
};

float radius = 0.4f;
int kernelSize = 16;

void main()
{
	vec4 FragPos = Camera.invProjectionViewMatrix * vec4(TexCoords * 2.0f - 1.0f, texture(gDepth, TexCoords).x, 1.0f); //FragPos is in camera space
	FragPos.xyzw /= FragPos.w;

	vec3 normal = normalize(texture(gNormal, TexCoords).xyz); //normal is in camera space

	vec2 noiseScale = vec2(Camera.SCR_WIDTH / 4.0f, Camera.SCR_HEIGHT / 4.0f);
	vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;
	
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	
	float occlusion = 0.0f;
	for(int i = 0; i < kernelSize; i++)
	{	
		//get sample position
		vec3 SAMPLE = TBN * samples[i].xyz; // from tangent to camera space
		SAMPLE = FragPos.xyz + SAMPLE * radius;
		
		vec4 offset = vec4(SAMPLE, 1.0f);
		offset = Camera.projectionViewMatrix * offset; // from camera to clip-space
		offset.xyzw /= offset.w;
		offset.xy = offset.xy * 0.5f + 0.5f;

		float sampleDepth = texture(gDepth, offset.xy).x;
		
		float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(offset.z - sampleDepth));
		occlusion += (sampleDepth >= offset.z ? 1.0f : 0.0f) * rangeCheck;
	}

	occlusion =  (occlusion / kernelSize);
	FragColor = vec4(vec3(occlusion), 1.0f);
}

#endif