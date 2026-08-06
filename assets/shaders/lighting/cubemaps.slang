#include "../include/Smoothie.slang"
#include "../include/lighting_functions.slang"

struct UniformData
{
    // float4x4 view;
    float4x4 projectView;
    float roughness;
}

struct Global_data
{
    TextureCube environmentMap;
    Texture2D skybox;
    ConstantBuffer<UniformData> data;
}

ParameterBlock<Smoothie_Globals> g_SmoothieGlobals;
ParameterBlock<Global_data> g_Globals;


[shader("vertex")]
float3 cubemaps_vertex(int vertexIndex: SV_VulkanVertexID, out float4 position: SV_Position)
{
    float3 TexCoords = aPos[vertexIndex];
    // float4x4 projectionMatrix = getProjectionMatrix(90.0f, 1.0f, 0.1f, 10.0f);
    position = mul(g_Globals.data.projectView, float4(aPos[vertexIndex], 1.0));
    return TexCoords;
}

[shader("fragment")]
float4 hdr_to_cubemap(float3 TexCoords)
{
    float2 uv = SampleSphericalMap(normalize(TexCoords));
    float3 color = g_Globals.skybox.Sample(g_SmoothieGlobals.samplers.Texture2DSampler, uv).rgb;
    return float4(color, 1.0);
}

[shader("fragment")]
float4 irradiance_map(float3 TexCoords)
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
            var _sampler = g_SmoothieGlobals.samplers.Texture2DSampler;
            irradiance += g_Globals.environmentMap.Sample(_sampler, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = pi * irradiance * (1.0 / float(nrSamples));
    return float4(irradiance, 1.0);
}

[shader("fragment")]
float4 prefilter_map(float3 TexCoords)
{
    float3 N = normalize(TexCoords);
    float3 R = N;
    float3 V = R;
    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;
    float3 prefilteredColor = float3(0.0);
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, g_Globals.data.roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V);
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            float D = DistributionGGX(N, H, g_Globals.data.roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

            float resolution = 512.0; // resolution of source cubemap (per face)
            float saTexel = 4.0 * pi / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = g_Globals.data.roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

            var _sampler = g_SmoothieGlobals.samplers.Texture2DSampler;
            prefilteredColor += g_Globals.environmentMap.SampleLevel(_sampler, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;
    return float4(prefilteredColor, 1.0);
}

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