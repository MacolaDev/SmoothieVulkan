#version 450

//Vertex shader
#ifdef SSHADER_VERTEX_STANDARD
#extension GL_ARB_shading_language_include: enable 
#include "../ShadersGeneral.glslh"

#define DESCRIPTOR_CAMERA MAKE_DESCRIPTOR_SET(0, 0)
#define DESCRIPTOR_GENERAL MAKE_DESCRIPTOR_SET(1, 0)
#include "../Descriptors.glslh"

layout (location = 0) in vec3 aPos; 
layout (location = 1) in uint aNormals;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in uint aTangents;
layout (location = 4) in uint aBitangents;
layout (location = 5) in uint aColor;

layout(set = 2, binding = 0, std140) uniform ModelData
{
	mat4 modelMatrix;
	uint ModelID;
};

vec4 unpackNormals(uint packedNormal)
{   
    float x = ((packedNormal >>  0)&0xFF);
    float y = ((packedNormal >>  8)&0xFF);
    float z = ((packedNormal >> 16)&0xFF);
    float w = ((packedNormal >> 24)&0xFF);

	vec4 result = vec4(x, y, z, w);

	result.xyz /= vec3(127);
	result.xyz -= vec3(1);

	return result; 
}


//Vertex shader output
layout (location = 0) out vec2 TexCoord;
layout (location = 1) out vec4 FragPos;
layout (location = 2) out float materialID;
layout (location = 3) out mat3 TBN;


const vec4 TreeWindSize = vec4(1000, 400, 100, 10);
const vec4 TreeWindTimer = vec4(1, 1, 1, 1) * 0.01;
const vec3 TreeWindPower = vec3(0.3, 0.1, 0); //wind effect on leafs, brances, bark

void main() 
{   
    
    TexCoord = aTexCoord;
	
	vec4 normals = unpackNormals(aNormals).xyzw;
	vec4 color = unpackNormals(aColor).xyzw;

	vec3 T = normalize(vec3(modelMatrix * vec4(unpackNormals(aTangents).xyz, 0.0f)));
	vec3 N = normalize(vec3(modelMatrix * vec4(unpackNormals(aNormals).xyz, 0.0f)));
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
    TBN = mat3(T, B, N); 

	
    vec4 WorldPos = modelMatrix * vec4(aPos, 1.0f);
    
	vec3 GlobalWind = (WorldPos.xyz/TreeWindSize.x) + TreeWindTimer.x * General.time;
	GlobalWind = vec3(cos(GlobalWind.x),0,sin(GlobalWind.z));
	GlobalWind.y = 1 + abs(dot(GlobalWind.x,GlobalWind.z));

	vec3 WindForce = (WorldPos.xyz/TreeWindSize.x) + TreeWindTimer.y * General.time - GlobalWind;
	WindForce = vec3(cos(WindForce.x), 0,sin(WindForce.y));

	vec3 WindAnimate = WindForce * TreeWindPower.x * (WorldPos.y/TreeWindSize.y);
	vec3 WindVariant = vec3(TreeWindTimer.z * General.time + (color.y/255.0 * 10));
	WindAnimate.xz += vec2(cos(WindVariant.x), sin(WindVariant.z)) * TreeWindPower.y * color.z/255.0;
	
	vec3 WindFlutter = (WorldPos.xyz / TreeWindSize.w)+(TreeWindTimer.w * General.time)+color.y/255.0;
	WindAnimate += vec3(abs(normalize(modelMatrix * normals)) * sin(dot(WindFlutter, vec3(1))) * TreeWindPower.z * (color.x/255.0 - 0.5));
	WorldPos += vec4(WindAnimate * GlobalWind.y, 0.0f);

	FragPos = WorldPos;
	gl_Position = Camera.projectionViewMatrix * WorldPos; 

	materialID = normals.w;
}
#endif


#ifdef SSHADER_FRAGMENT_STANDARD

//Vertex shader input
layout (location = 0) in vec2 TexCoord;
layout (location = 1) in vec4 FragPos;
layout (location = 2) in float materialID;
layout (location = 3) in mat3 TBN;


//Fragment shader output
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gMRAO;


layout(set = 2, binding = 1) uniform sampler2D diffuseTexture;
layout(set = 2, binding = 2) uniform sampler2D roughnessTexture;
layout(set = 2, binding = 3) uniform sampler2D normalTexture;

void main()
{
	vec2 UV = vec2(TexCoord.x/3.0f + materialID/3.0f, TexCoord.y);

	vec4 color = texture(diffuseTexture, UV);
	float roughness = texture(roughnessTexture, UV).x;
	vec3 normal = texture(normalTexture, UV).xyz;

	if(color.w < 0.5f) discard;
	gAlbedoSpec = vec4(color.rgb, 1.0f);
	vec4 MRAO = vec4(0.0f, roughness, 1.0f, 1.0f);
	
	normal = normalize(normal * 2 - 1);
	gNormal = vec4(abs(normalize(TBN * normal)), 1);
	
	gMRAO = MRAO;
	gPosition = FragPos;
	
}
#endif