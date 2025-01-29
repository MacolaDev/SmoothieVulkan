#version 450

layout(set = 0, binding = 0) uniform UnifomBuffer
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 viewPos;
    mat4 projectionViewMatrix;
} Matrices;

//Vertex shader
#ifdef VERTEX_SHADER

//Vertex shader input data
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

//Model Matrices
struct ModelMatrices
{
	mat4 modelMatrix;
	mat4 normalMatrix;
};

layout(set = 2, binding = 0, std430) readonly buffer ModelData
{
	ModelMatrices modelMatrices[];
};

//Vertex shader output
layout (location = 0) out vec2 TexCoord;
layout (location = 1) out vec4 FragPos;
layout (location = 2) out mat3 TBN;

void main() 
{   
    
    TexCoord = aTexCoord;
    FragPos = modelMatrices[gl_InstanceIndex].modelMatrix * vec4(aPos, 1.0f);
	
	vec3 T = normalize(mat3(modelMatrices[gl_InstanceIndex].normalMatrix) * unpackNormals(aTangents).xyz);
	vec3 B = normalize(mat3(modelMatrices[gl_InstanceIndex].normalMatrix) * unpackNormals(aBitangents).xyz);
	vec3 N = normalize(mat3(modelMatrices[gl_InstanceIndex].normalMatrix) * unpackNormals(aNormals).xyz);

    TBN = mat3(T, B, N); 
    gl_Position = Matrices.projectionViewMatrix * FragPos; 

}
#endif


#ifdef FRAGMENT_SHADER

//Vertex shader input
layout (location = 0) in vec2 TexCoord;
layout (location = 1) in vec4 FragPos;
layout (location = 2) in mat3 TBN;

//Input textures
layout(set = 2, binding = 1) uniform sampler2D diffuse;
layout(set = 2, binding = 2) uniform sampler2D normalMap;
layout(set = 2, binding = 3) uniform sampler2D metalnessMap;
layout(set = 2, binding = 4) uniform sampler2D roughnessMap;
layout(set = 2, binding = 5) uniform sampler2D AOMap;

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
	gMRAO.b = texture(AOMap, TexCoord).r;
	
	gPosition = FragPos;
	
	vec3 normal = texture(normalMap, TexCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	gNormal = vec4(normalize(TBN * normal), 1);
	
	gAlbedoSpec.rgba = texture(diffuse, TexCoord).rgba;
	
}
#endif