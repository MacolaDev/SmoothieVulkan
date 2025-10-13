#version 450

#ifdef SSHADER_VERTEX_PBS_VERTEX
vec3 aPos[36] = vec3[](
            vec3(-1.0f, -1.0f, -1.0f),  // bottom-left
            vec3(1.0f,  1.0f, -1.0f),  // top-right
            vec3(1.0f, -1.0f, -1.0f),  // bottom-right         
            vec3(1.0f,  1.0f, -1.0f),  // top-right
            vec3(-1.0f, -1.0f, -1.0f),  // bottom-left
            vec3(-1.0f,  1.0f, -1.0f),  // top-left
            // front face
            vec3(-1.0f, -1.0f,  1.0f),  // bottom-left
            vec3( 1.0f, -1.0f,  1.0f),  // bottom-right
            vec3( 1.0f,  1.0f,  1.0f),  // top-right
            vec3( 1.0f,  1.0f,  1.0f),  // top-right
            vec3(-1.0f,  1.0f,  1.0f),  // top-left
            vec3(-1.0f, -1.0f,  1.0f),  // bottom-left
            // left face
            vec3(-1.0f,  1.0f,  1.0f),  // top-right
            vec3(-1.0f,  1.0f, -1.0f),  // top-left
            vec3(-1.0f, -1.0f, -1.0f),  // bottom-left
            vec3(-1.0f, -1.0f, -1.0f),  // bottom-left
            vec3(-1.0f, -1.0f,  1.0f),  // bottom-right
            vec3(-1.0f,  1.0f,  1.0f),  // top-right
            // right face
            vec3(1.0f,  1.0f,  1.0f),   // top-left
            vec3(1.0f, -1.0f, -1.0f),   // bottom-right
            vec3(1.0f,  1.0f, -1.0f),   // top-right         
            vec3(1.0f, -1.0f, -1.0f),   // bottom-right
            vec3(1.0f,  1.0f,  1.0f),   // top-left
            vec3(1.0f, -1.0f,  1.0f),   // bottom-left     
            // bottom face
            vec3(-1.0f, -1.0f, -1.0f),   // top-right
            vec3( 1.0f, -1.0f, -1.0f),   // top-left
            vec3( 1.0f, -1.0f,  1.0f),   // bottom-left
            vec3( 1.0f, -1.0f,  1.0f),   // bottom-left
            vec3(-1.0f, -1.0f,  1.0f),   // bottom-right
            vec3(-1.0f, -1.0f, -1.0f),   // top-right
            // top face
            vec3(-1.0f,  1.0f, -1.0f),   // top-left
            vec3( 1.0f,  1.0f , 1.0f),   // bottom-right
            vec3( 1.0f,  1.0f, -1.0f),   // top-right     
            vec3( 1.0f,  1.0f,  1.0f),   // bottom-right
            vec3(-1.0f,  1.0f, -1.0f),   // top-left
            vec3(-1.0f,  1.0f,  1.0f)   // bottom-left   
);

layout (location = 0) out vec3 TexCoords;
layout(push_constant, std430) uniform pc
{
	mat4 view;
} Matrices;

const float pi = 3.141592;

mat4 getProjectionMatrix(float fovy, float aspec, float zNear, float zFar)
{
    mat4 matrix = mat4(0.0f);
    matrix[0][0] = 1.0f;
    matrix[1][1] = 1.0f;
    matrix[2][2] = 1.0f;
    matrix[3][3] = 1.0f;

    float toRadians = fovy*pi/180;
	float f = 1/tan(toRadians/2);
	matrix[0][0] = f / aspec;
	matrix[1][1] = f;
	float value = zNear - zFar;

	matrix[2][2] = (zFar + zNear) / value;

	matrix[3][2] = (2 * zFar * zNear) / value;
	matrix[2][3] = -1.0f;
	matrix[3][3] = 0.0f;
    return matrix;

}

void main()
{
	TexCoords = aPos[gl_VertexIndex];
	mat4 projectionMatrix = getProjectionMatrix(90.0f, 1.0f, 0.1f, 10.0f);
    gl_Position = projectionMatrix * Matrices.view * vec4(aPos[gl_VertexIndex], 1.0);
}
#endif


//************** IRADINACE MAP ************//
#ifdef SSHADER_FRAGMENT_IRRADIANCE_MAP
layout(location = 0) in vec3 TexCoords;
layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 0) uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{
	vec3 N = normalize(TexCoords);

	vec3 irradiance = vec3(0.0);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, N);
	up = cross(N, right);

	float sampleDelta = 0.025;
	float nrSamples = 0.0;
	
	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// spherical to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
			irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));
	FragColor = vec4(irradiance, 1.0);
}
#endif

//************** HDR TO CUBEMAP ***************//
#ifdef SSHADER_FRAGMENT_HDR_TO_CUBEMAP
layout (location = 0) in vec3 TexCoords;
layout (location = 0) out vec4 FragColor;

layout (set = 0, binding = 0) uniform sampler2D skybox;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

void main()
{
	vec2 uv = SampleSphericalMap(normalize(TexCoords));
	vec3 color = texture(skybox, uv).rgb;
	FragColor = vec4(color, 1.0);
}
#endif


//********** BRDF ***********//
#ifdef SSHADER_FRAGMENT_BRDF
layout (location = 0) in vec2 TexCoords;
layout (location = 0) out vec2 FragColor;

const float PI = 3.14159265359;
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	// from spherical coordinates to cartesian coordinates
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float VanDerCorpus(uint n, uint base)
{
	float invBase = 1.0 / float(base);
	float denom = 1.0;
	float result = 0.0;
	for(uint i = 0u; i < 32u; ++i)
	{
		if(n > 0u){
			denom = mod(float(n), 2.0);
			result += denom * invBase;
			invBase = invBase / 2.0;
			n = uint(float(n) / 2.0);
		}
	}
	return result;
}

vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), VanDerCorpus(i, 2u));
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float a = roughness;
	float k = (a * a) / 2.0;
	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	return ggx1 * ggx2;
}

vec2 IntegrateBRDF(float NdotV, float roughness)
{
	vec3 V;
	V.x = sqrt(1.0 - NdotV*NdotV);
	V.y = 0.0;
	V.z = NdotV;
	float A = 0.0;
	float B = 0.0;
	vec3 N = vec3(0.0, 0.0, 1.0);
	const uint SAMPLE_COUNT = 1024u;
	for(uint i = 0u; i < SAMPLE_COUNT; ++i){
		vec2 Xi = Hammersley(i, SAMPLE_COUNT);
		vec3 H = ImportanceSampleGGX(Xi, N, roughness);
		vec3 L = normalize(2.0 * dot(V, H) * H - V);
		float NdotL = max(L.z, 0.0);
		float NdotH = max(H.z, 0.0);
		float VdotH = max(dot(V, H), 0.0);
	
	if(NdotL > 0.0)
	{
		float G = GeometrySmith(N, V, L, roughness);
		float G_Vis = (G * VdotH) / (NdotH * NdotV);
		float Fc = pow(1.0 - VdotH, 5.0);
		A += (1.0 - Fc) * G_Vis;
		B += Fc * G_Vis;}	
	}

	A /= float(SAMPLE_COUNT);
	B /= float(SAMPLE_COUNT);
	return vec2(A, B);
}

void main()
{
	vec2 integratedBRDF = IntegrateBRDF(TexCoords.x, TexCoords.y);
	FragColor = integratedBRDF;
}
#endif


//************ PREFILTER *****************//
#ifdef SSHADER_FRAGMENT_PREFILTER
layout (location = 0) in vec3 TexCoords;
layout (location = 0) out vec4 FragColor;

layout (set = 0, binding = 0) uniform samplerCube environmentMap;

layout(push_constant, std430) uniform pc
{
	mat4 view;
    float roughness;
};

const float PI = 3.14159265359;

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	// from spherical coordinates to cartesian coordinates
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float VanDerCorpus(uint n, uint base)
{
	float invBase = 1.0 / float(base);
	float denom = 1.0;
	float result = 0.0;
	for(uint i = 0u; i < 32u; ++i)
	{
		if(n > 0u){
			denom = mod(float(n), 2.0);
			result += denom * invBase;
			invBase = invBase / 2.0;
			n = uint(float(n) / 2.0);
		}
	}
	return result;
}

vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), VanDerCorpus(i, 2u));
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

void main()
{
	vec3 N = normalize(TexCoords);
	vec3 R = N;
	vec3 V = R;
	const uint SAMPLE_COUNT = 1024u;
	float totalWeight = 0.0;
	vec3 prefilteredColor = vec3(0.0);
	for(uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		vec2 Xi = Hammersley(i, SAMPLE_COUNT);
		vec3 H = ImportanceSampleGGX(Xi, N, roughness);
		vec3 L = normalize(2.0 * dot(V, H) * H - V);
		float NdotL = max(dot(N, L), 0.0);
		if(NdotL > 0.0)
		{
			// sample from the environment's mip level based on roughness/pdf
            float D   = DistributionGGX(N, H, roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

            float resolution = 512.0; // resolution of source cubemap (per face)
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
            
            prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
		}
	}
	prefilteredColor = prefilteredColor / totalWeight;
	FragColor = vec4(prefilteredColor, 1.0);

}
#endif