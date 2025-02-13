#version 450

//Vertex shader
#ifdef VERTEX_SHADER
layout (location = 0) in vec3 aPos; 
layout (location = 1) in uint aNormals;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in uint aTangents;
layout (location = 4) in uint aBitangents;
layout (location = 5) in uint aColor;

layout(set = 0, binding = 0) uniform UnifomBuffer
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 viewPos;
    mat4 projectionViewMatrix;
} Matrices;

layout (set = 1, binding = 0) uniform Standard2
{
    int SCR_WIDTH;
	int SCR_HEIGHT;
	int time;
};


struct ModelMatrices
{
	mat4 modelMatrix;
	mat4 normalMatrix;
};

layout(set = 2, binding = 0, std430) readonly buffer ModelData
{
	ModelMatrices modelMatrices[];
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
layout (location = 1) out vec2 TexCoord;
layout (location = 2) out vec4 FragPos;
layout (location = 3) out vec2 bark_uv;
layout (location = 4) out vec2 stump_uv;
layout (location = 5) out vec2 leaf_uv;
layout (location = 6) out mat3 TBN;


const vec4 TreeWindSize = vec4(1000, 400, 100, 10);
const vec4 TreeWindTimer = vec4(1, 1, 1, 1) * 0.01;
const vec3 TreeWindPower = vec3(0.3, 0.1, 0); //wind effect on leafs, brances, bark

void main() 
{   
    
    TexCoord = aTexCoord;
	
	vec4 normals = unpackNormals(aNormals).xyzw;
	vec4 color = unpackNormals(aColor).xyzw;

	vec3 T = normalize(mat3(modelMatrices[gl_InstanceIndex].normalMatrix) * unpackNormals(aTangents).xyz);
	vec3 B = normalize(mat3(modelMatrices[gl_InstanceIndex].normalMatrix) * unpackNormals(aBitangents).xyz);
	vec3 N = normalize(mat3(modelMatrices[gl_InstanceIndex].normalMatrix) * normals.xyz);
    TBN = mat3(T, B, N);

	
    vec4 WorldPos = modelMatrices[gl_InstanceIndex].modelMatrix * vec4(aPos, 1.0f);
    
	vec3 GlobalWind = (WorldPos.xyz/TreeWindSize.x) + TreeWindTimer.x * time;
	GlobalWind = vec3(cos(GlobalWind.x),0,sin(GlobalWind.z));
	GlobalWind.y = 1 + abs(dot(GlobalWind.x,GlobalWind.z));

	vec3 WindForce = (WorldPos.xyz/TreeWindSize.x) + TreeWindTimer.y * time - GlobalWind;
	WindForce = vec3(cos(WindForce.x),0,sin(WindForce.y));

	vec3 WindAnimate = WindForce * TreeWindPower.x * (WorldPos.y/TreeWindSize.y);
	vec3 WindVariant = vec3(TreeWindTimer.z * time + (color.y/255.0 * 10));
	WindAnimate.xz += vec2(cos(WindVariant.x), sin(WindVariant.z)) * TreeWindPower.y * color.z/255.0;
	
	vec3 WindFlutter = (WorldPos.xyz / TreeWindSize.w)+(TreeWindTimer.w * time)+color.y/255.0;
	WindAnimate += vec3(abs(normalize(modelMatrices[gl_InstanceIndex].modelMatrix * normals)) * sin(dot(WindFlutter, vec3(1))) * TreeWindPower.z * (color.x/255.0 - 0.5));
	WorldPos += vec4(WindAnimate * GlobalWind.y, 0.0f);

	FragPos = WorldPos;
	gl_Position = Matrices.projectionViewMatrix * WorldPos; 

	bark_uv = vec2(0, 0);
	stump_uv = vec2(0, 0);
	leaf_uv = vec2(0, 0);

	if(normals.w == 0)
	{
		bark_uv = aTexCoord;
	}
	
	if(normals.w == 1)
	{
		stump_uv = aTexCoord;
	}
	
	if(normals.w == 2)
	{
		leaf_uv = aTexCoord;
	}
}
#endif


#ifdef FRAGMENT_SHADER

//Vertex shader input
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec4 FragPos;
layout (location = 3) in vec2 bark_uv;
layout (location = 4) in vec2 stump_uv;
layout (location = 5) in vec2 leaf_uv;
layout (location = 6) in mat3 TBN;


//Fragment shader output
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gMRAO;


layout(set = 2, binding = 1) uniform sampler2D bark_diffuse;
layout(set = 2, binding = 2) uniform sampler2D bark_roughness;
layout(set = 2, binding = 3) uniform sampler2D bark_normal;

layout(set = 2, binding = 4) uniform sampler2D stump_diffuse;
layout(set = 2, binding = 5) uniform sampler2D stump_roughness;
layout(set = 2, binding = 6) uniform sampler2D stump_normal;

layout(set = 2, binding = 7) uniform sampler2D leaf_diffuse;
layout(set = 2, binding = 8) uniform sampler2D leaf_roughness;
layout(set = 2, binding = 9) uniform sampler2D leaf_normal;


bool isTextureActive(vec2 data)
{
	if((data.x == 0) && (data.y == 0))
	{
		return false;
	}
	return true;
	
}

void main()
{
	vec4 color;
	vec3 normal;
	vec3 roughness;
	
	if(isTextureActive(bark_uv))
	{
		color = texture(bark_diffuse, bark_uv).xyzw;
		normal = texture(bark_normal, bark_uv).xyz;
		roughness = texture(bark_roughness, bark_uv).xyz;
		
	}
	
	if(isTextureActive(stump_uv))
	{
		color = texture(stump_diffuse, stump_uv).xyzw;
		normal = texture(stump_normal, stump_uv).xyz;
		roughness = texture(stump_roughness, stump_uv).xyz;
	}
	
	if(isTextureActive(leaf_uv))
	{
		
		color = texture(leaf_diffuse, leaf_uv).xyzw;
		normal = texture(leaf_normal, leaf_uv).xyz;
		roughness = texture(leaf_roughness, leaf_uv).xyz;
	}
	if(color.w < 0.1){ discard;}

	gAlbedoSpec = color;
	
	vec4 MRAO = vec4(0.0f, roughness.r, 1.0f, 1.0f);
	
	normal = normalize(normal * 2 - 1);
	gNormal = vec4(abs(normalize(TBN * normal)), 1);
	
	gMRAO = MRAO;
	
	gPosition = FragPos;
	
}
#endif