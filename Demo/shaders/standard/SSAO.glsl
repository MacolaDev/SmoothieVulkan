#version 450

//Fragment shader
#ifdef FRAGMENT_SHADER


in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform vec3 samples[64];

float radius = 0.2f;
float bias = 0.055;
int kernelSize = 64;

layout (std140, binding = 0) uniform Standard
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
	vec3 viewPos;
};

layout (std140, binding = 1) uniform Standard2
{
    int SCR_WIDTH;
	int SCR_HEIGHT;
};


const vec2 noiseScale = vec2(SCR_WIDTH/4.0, SCR_HEIGHT/4.0); 

out vec4 FragColor;
void main()
{
	vec3 FragPos = vec3(viewMatrix * vec4(texture(gPosition, TexCoords).xyz, 1.0f)); //FragPos is in camera space
	vec3 normal = vec3(viewMatrix * vec4(texture(gNormal, TexCoords).xyz, 1.0f)); //normal is in camera space
	normal = normalize(normal);

	vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;
	
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	
	float occlusion = 0.0f;
	for(int i = 0; i < kernelSize; i++)
	{	
		//get sample position
		vec3 SAMPLE = TBN * samples[i]; // from tangent to camera space
		SAMPLE = FragPos + SAMPLE * radius;

		vec4 offset = vec4(SAMPLE, 1.0f);
		offset = projectionMatrix * offset; // from camera to clip-space
		offset.xyz /= offset.w; // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
		float sampleDepth =  vec3(viewMatrix * vec4(texture(gPosition, offset.xy).xyz, 1.0f)).z; //
		
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(FragPos.z - sampleDepth));
		occlusion += (sampleDepth >= SAMPLE.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	occlusion = 1 - (occlusion / kernelSize);
	FragColor = vec4(vec3(occlusion), 1);
}

#endif


//Vertex shader
#ifdef VERTEX_SHADER
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
out vec2 TexCoords;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
#endif