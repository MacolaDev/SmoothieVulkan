static const float3 aPos[36] = 
{
    float3(-1.0f, -1.0f, -1.0f), // bottom-left
    float3(1.0f, 1.0f, -1.0f),   // top-right
    float3(1.0f, -1.0f, -1.0f),  // bottom-right
    float3(1.0f, 1.0f, -1.0f),   // top-right
    float3(-1.0f, -1.0f, -1.0f), // bottom-left
    float3(-1.0f, 1.0f, -1.0f),  // top-left
    // front face
    float3(-1.0f, -1.0f, 1.0f), // bottom-left
    float3(1.0f, -1.0f, 1.0f),  // bottom-right
    float3(1.0f, 1.0f, 1.0f),   // top-right
    float3(1.0f, 1.0f, 1.0f),   // top-right
    float3(-1.0f, 1.0f, 1.0f),  // top-left
    float3(-1.0f, -1.0f, 1.0f), // bottom-left
    // left face
    float3(-1.0f, 1.0f, 1.0f),   // top-right
    float3(-1.0f, 1.0f, -1.0f),  // top-left
    float3(-1.0f, -1.0f, -1.0f), // bottom-left
    float3(-1.0f, -1.0f, -1.0f), // bottom-left
    float3(-1.0f, -1.0f, 1.0f),  // bottom-right
    float3(-1.0f, 1.0f, 1.0f),   // top-right
    // right face
    float3(1.0f, 1.0f, 1.0f),   // top-left
    float3(1.0f, -1.0f, -1.0f), // bottom-right
    float3(1.0f, 1.0f, -1.0f),  // top-right
    float3(1.0f, -1.0f, -1.0f), // bottom-right
    float3(1.0f, 1.0f, 1.0f),   // top-left
    float3(1.0f, -1.0f, 1.0f),  // bottom-left
    // bottom face
    float3(-1.0f, -1.0f, -1.0f), // top-right
    float3(1.0f, -1.0f, -1.0f),  // top-left
    float3(1.0f, -1.0f, 1.0f),   // bottom-left
    float3(1.0f, -1.0f, 1.0f),   // bottom-left
    float3(-1.0f, -1.0f, 1.0f),  // bottom-right
    float3(-1.0f, -1.0f, -1.0f), // top-right
    // top face
    float3(-1.0f, 1.0f, -1.0f), // top-left
    float3(1.0f, 1.0f, 1.0f),   // bottom-right
    float3(1.0f, 1.0f, -1.0f),  // top-right
    float3(1.0f, 1.0f, 1.0f),   // bottom-right
    float3(-1.0f, 1.0f, -1.0f), // top-left
    float3(-1.0f, 1.0f, 1.0f)   // bottom-left
};

static const float pi = 3.14159265359;

float4x4 getProjectionMatrix(float fovy, float aspec, float zNear, float zFar)
{
    float4x4 matrix = float4x4(0.0f);
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
    return transpose(matrix);
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;
    float phi = 2.0 * pi * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    // from spherical coordinates to cartesian coordinates
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

float VanDerCorpus(uint n, uint base)
{
    float invBase = 1.0 / float(base);
    float denom = 1.0;
    float result = 0.0;
    for (uint i = 0u; i < 32u; ++i)
    {
		
        if (n > 0u) {
            denom = float(n) % 2.0;
            result += denom * invBase;
            invBase = invBase / 2.0;
            n = uint(float(n) / 2.0);
        }
    }
    return result;
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), VanDerCorpus(i, 2u));
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0;
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

float2 IntegrateBRDF(float NdotV, float roughness)
{
    float3 V;
    V.x = sqrt(1.0 - NdotV * NdotV);
    V.y = 0.0;
    V.z = NdotV;
    float A = 0.0;
    float B = 0.0;
    float3 N = float3(0.0, 0.0, 1.0);
    const uint SAMPLE_COUNT = 1024u;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V);
        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if (NdotL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);
            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;}
    }

    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return float2(A, B);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = pi * denom * denom;

    return nom / denom;
}

[vk_push_constant]
cbuffer PBS_VERTEX_push_constant
{
    float4x4 PBS_VERTEX_view;
}

[vk_push_constant]
cbuffer fragment_PREFILTER_PC
{
    float4x4 view;
    float roughness;
}

[vk_binding(0, 0)]
SamplerCube environmentMap;

[shader("vertex")]
float3 vertex_PBS_VERTEX(int vertexIndex : SV_VulkanVertexID, out float4 position: SV_Position)
{
    float3 TexCoords = aPos[vertexIndex];
    float4x4 projectionMatrix = getProjectionMatrix(90.0f, 1.0f, 0.1f, 10.0f);
    position = mul(mul(projectionMatrix, PBS_VERTEX_view), float4(aPos[vertexIndex], 1.0));
    return TexCoords;
}

[shader("fragment")]
float4 fragment_IRRADIANCE_MAP(float3 TexCoords)
{
	float3 N = normalize(TexCoords);
	float3 irradiance = float3(0.0);
	float3 up = float3(0.0, 1.0, 0.0);
	float3 right = cross(up, N);
	up = cross(N, right);
	float sampleDelta = 0.025;
	float nrSamples = 0.0;
    for (float phi = 0.0; phi < 2.0 * pi; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * pi; theta += sampleDelta)
		{
			// spherical to cartesian (in tangent space)
			float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            irradiance += environmentMap.Sample(sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
    }
    irradiance = pi * irradiance * (1.0 / float(nrSamples));
	return float4(irradiance, 1.0);
}

[shader("fragment")]
float2 fragment_BRDF(float2 TexCoords)
{
	float2 integratedBRDF = IntegrateBRDF(TexCoords.x, TexCoords.y);
	return integratedBRDF;
}

[shader("fragment")]
float4 fragment_PREFILTER(float3 TexCoords)
{
	float3 N = normalize(TexCoords);
	float3 R = N;
	float3 V = R;
	const uint SAMPLE_COUNT = 1024u;
	float totalWeight = 0.0;
	float3 prefilteredColor = float3(0.0);
	for(uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 H = ImportanceSampleGGX(Xi, N, roughness);
		float3 L = normalize(2.0 * dot(V, H) * H - V);
		float NdotL = max(dot(N, L), 0.0);
		if(NdotL > 0.0)
		{
			// sample from the environment's mip level based on roughness/pdf
            float D   = DistributionGGX(N, H, roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

            float resolution = 512.0; // resolution of source cubemap (per face)
            float saTexel  = 4.0 * pi / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

            prefilteredColor += environmentMap.SampleLevel(L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
		}
	}
	prefilteredColor = prefilteredColor / totalWeight;
	return float4(prefilteredColor, 1.0);
}
