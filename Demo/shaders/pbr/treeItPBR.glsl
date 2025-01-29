#version 450

//Vertex shader
#ifdef VERTEX_SHADER
layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec4 aNormals;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 aTangents;
layout (location = 4) in vec4 aBitangents;
layout (location = 5) in vec4 aColor;

layout (std140, binding = 0) uniform Standard
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
	vec3 viewPos;
    mat4 projectionViewMatrix;
};


layout (std140, binding = 1) uniform Standard2
{
    int SCR_WIDTH;
	int SCR_HEIGHT;
	int time;
};

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

vec3 unpackNormals(vec4 packedNormal)
{   
    float x = packedNormal.x/127 - 1;
    float y = packedNormal.y/127 - 1;
    float z = packedNormal.z/127 - 1;
    return vec3(x, -z, y); 
}

out vec2 TexCoord;
out vec3 vertexNormals;
out vec4 FragPos;
out mat3 TBN;

out vec2 bark_uv;
out vec2 stump_uv;
out vec2 leaf_uv;


uniform vec4 TreeWindSize = vec4(1000, 400, 100, 10);
uniform vec4 TreeWindTimer = vec4(1, 1, 1, 1) * 0.01;
uniform vec3 TreeWindPower = vec3(0.3, 0.1, 0); //wind effect on leafs, brances, bark


void main() 
{   
    
    TexCoord = aTexCoord;
    vertexNormals =  normalize(normalMatrix * vec3(unpackNormals(aNormals)));

    //T, B and N should be multipled with normal matrix 
    vec3 T = normalize(vec3(modelMatrix * vec4(normalMatrix * unpackNormals(aTangents), 0.0)));
    vec3 B = normalize(vec3(modelMatrix * vec4(normalMatrix * unpackNormals(aBitangents), 0.0)));
    vec3 N = normalize(vec3(modelMatrix * vec4(normalMatrix * unpackNormals(aNormals), 0.0)));
    TBN = mat3(T, B, N);
	
	
    vec4 WorldPos = modelMatrix * vec4(aPos, 1.0f);
    
	vec3 GlobalWind = (WorldPos.xyz/TreeWindSize.x) + TreeWindTimer.x * time;
	GlobalWind = vec3(cos(GlobalWind.x),0,sin(GlobalWind.z));
	GlobalWind.y = 1 + abs(dot(GlobalWind.x,GlobalWind.z));

	vec3 WindForce = (WorldPos.xyz/TreeWindSize.x) + TreeWindTimer.y * time - GlobalWind;
	WindForce = vec3(cos(WindForce.x),0,sin(WindForce.y));

	vec3 WindAnimate = WindForce * TreeWindPower.x * (WorldPos.y/TreeWindSize.y);
	vec3 WindVariant = vec3(TreeWindTimer.z * time + (aColor.y/255.0 * 10));
	WindAnimate.xz += vec2(cos(WindVariant.x), sin(WindVariant.z)) * TreeWindPower.y * aColor.z/255.0;
	
	vec3 WindFlutter = (WorldPos.xyz / TreeWindSize.w)+(TreeWindTimer.w * time)+aColor.y/255.0;
	WindAnimate += vec3(abs(normalize(modelMatrix * aNormals)) * sin(dot(WindFlutter, vec3(1))) * TreeWindPower.z * (aColor.x/255.0 - 0.5));
	WorldPos += vec4(WindAnimate * GlobalWind.y, 0.0f);

	FragPos = WorldPos;
	gl_Position = projectionViewMatrix * WorldPos; 
	
	bark_uv = vec2(0, 0);
	stump_uv = vec2(0, 0);
	leaf_uv = vec2(0, 0);

	if(aNormals.w == 0)
	{
		bark_uv = aTexCoord;
	}
	
	if(aNormals.w == 1)
	{
		stump_uv = aTexCoord;
	}
	
	if(aNormals.w == 2)
	{
		leaf_uv = aTexCoord;
	}
}
#endif




//Fragment shader
#ifdef FRAGMENT_SHADER
in vec2 TexCoord;
in vec4 FragPos;
in vec3 vertexNormals;
in mat3 TBN;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gMRAO;

uniform sampler2D bark_diffuse;
uniform sampler2D bark_roughness;
uniform sampler2D bark_normal;

uniform sampler2D stump_diffuse;
uniform sampler2D stump_roughness;
uniform sampler2D stump_normal;

uniform sampler2D leaf_diffuse;
uniform sampler2D leaf_roughness;
uniform sampler2D leaf_normal;

in vec2 bark_uv;
in vec2 stump_uv;
in vec2 leaf_uv;

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