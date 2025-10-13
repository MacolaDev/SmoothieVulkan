#version 450

#extension GL_ARB_shading_language_include: enable 
#include "../ShadersGeneral.glslh"


#define DESCRIPTOR_CAMERA MAKE_DESCRIPTOR_SET(0, 0)
#define DESCRIPTOR_GENERAL MAKE_DESCRIPTOR_SET(1, 0)
#include "../Descriptors.glslh"
#ifdef SSHADER_VERTEX_STANDARD
layout (location = 0) in vec3 aPos; 
layout (location = 1) in uint aNormals;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in uint aTangents;
layout (location = 4) in uint aBitangents;

vec4 unpackNormals(uint packedNormal)
{   
    float x = ((packedNormal >>  0)&0xFF);
    float y = ((packedNormal >>  8)&0xFF);
    float z = ((packedNormal >> 16)&0xFF);
    float w = ((packedNormal >> 24)&0xFF);

	vec4 result = vec4(x, y, z, w);

	result /= vec4(127);
	result -= vec4(1);

	return result; 
}

layout(set = 2, binding = 0, std140) uniform ModelData
{
	mat4 modelMatrix;
	mat4 normalMatrix;
	uint ModelID;
};

//Vertex shader output
layout (location = 0) out vec2 TexCoord;
layout (location = 1) out vec4 FragPos;
layout (location = 2) out mat3 TBN;

void main() 
{   
	
	TexCoord = aTexCoord;
    FragPos = modelMatrix * vec4(aPos, 1.0f);
	vec3 T = normalize(vec3(modelMatrix * vec4(unpackNormals(aTangents).xyz, 0.0f)));
	vec3 N = normalize(vec3(modelMatrix * vec4(unpackNormals(aNormals).xyz, 0.0f)));
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
    TBN = mat3(T, B, N); 
    gl_Position = Camera.projectionViewMatrix * FragPos; 
}
#endif

#ifdef SSHADER_FRAGMENT_STANDARD

//Vertex shader input
layout (location = 0) in vec2 TexCoord;
layout (location = 1) in vec4 FragPos;
layout (location = 2) in mat3 TBN;

//Input textures
layout(set = 2, binding = 1) uniform sampler2D diffuse;
layout(set = 2, binding = 2) uniform sampler2D normalMap;
layout(set = 2, binding = 3) uniform sampler2D metalnessMap;
layout(set = 2, binding = 4) uniform sampler2D roughnessMap;

//gBuffer output
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gMRAO;

void main()
{
	gMRAO = vec4(0.0f, 0.0f, 1.0f, 1.0f);

	gMRAO.r = texture(metalnessMap, TexCoord).r;
	gMRAO.g = texture(roughnessMap, TexCoord).r;
	gMRAO.b = 1.0f;

	gPosition = FragPos;

	
	vec3 normal = texture(normalMap, TexCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	gNormal = vec4(normalize(TBN * normal), 1);
	
	gAlbedoSpec.rgba = texture(diffuse, TexCoord).rgba;
	
}
#endif

#ifdef SSHADER_VERTEX_STANDARD_SHADOW
layout (location = 0) in vec3 aPos; 
layout (location = 1) in uint aNormals;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in uint aTangents;
layout (location = 4) in uint aBitangents;


layout(set = 2, binding = 0, std140) uniform ModelData
{
	mat4 modelMatrix;
	mat4 normalMatrix;
	uint ModelID;
};


void main() 
{   
    vec4 FragPos = modelMatrix * vec4(aPos, 1.0f); 
    gl_Position = Camera.projectionViewMatrix * FragPos; 
}
#endif

#ifdef SSHADER_FRAGMENT_STANDARD_SHADOW
void main()
{

}
#endif