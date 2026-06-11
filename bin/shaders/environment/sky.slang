#include "../Smoothie.slang"

struct Sky
{
    TextureCube environmentMap;
}

ParameterBlock<Smoothie_Globals> g_GlobalData;
ParameterBlock<Sky> g_Sky;


[shader("vertex")]
float3 vertex_skybox(uint vertexId: SV_VulkanVertexID, out float4 position: SV_Position)
{
    var _camera = g_GlobalData.camera;
    var viewMatrix = g_GlobalData.camera.viewMatrix;

    float4x4 rotView = float4x4
    (
        float4(viewMatrix[0].xyz, 0.0f),
        float4(viewMatrix[1].xyz, 0.0f),
        float4(viewMatrix[2].xyz, 0.0f),
        float4(viewMatrix[3].xyz, 1.0f)
    );
    
    position = mul(mul(_camera.projectionMatrix, rotView), float4(aPos[vertexId], 1.0)).xyww;
    return aPos[vertexId];
}


[shader("fragment")]
float4 fragment_skybox(float3 TexCoords)
{
    // return float4(g_Atmosphere.environmentMap.Sample(g_GlobalData.samplers.Texture2DSampler, TexCoords).xyz, 1.0f);
    return float4(0.5 * normalize(TexCoords) + 0.5, 1.0f);
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






//layout (set = 0, binding = 0) uniform Standard
//{
//    mat4 projectionMatrix;
//    mat4 viewMatrix;
//	vec3 viewPos;
//	mat4 projectionViewMatrix;
//} Matrices;
//
//const float PI = 3.14159265359;
//const float HR = 8.0;   // Rayleigh scale height
//const float HM = 4.2;   // Mie scale height
//
//// These values are usually derived from physical constants.
//const vec3 betaR = vec3(5.8e-6, 13.5e-6, 33.1e-6); // Rayleigh scattering coefficients
//const vec3 betaM = vec3(21e-6);                     // Mie scattering coefficient (assumed equal for R, G, B)
//
//// Henyey-Greenstein anisotropy factor for Mie scattering
//const float g = 0.8;
//
//// Number of samples for integration along the ray
//const int NUM_SAMPLES = 16;
//
//float rayleighPhase(float cosTheta) {
//    return (3.0 / (16.0 * PI)) * (1.0 + cosTheta * cosTheta);
//}
//
//float miePhase(float cosTheta) {
//    float g2 = g * g;
//    return (1.0 / (4.0 * PI)) * ((1.0 - g2) / pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
//}
//
//const vec3 sunDirection = normalize(vec3(1.0f));
//
//vec3 calulcateLight()
//{
//    // Compute view direction (assuming a fullscreen quad in clip space)
//    // For a more robust solution, derive the ray from the inverse projection matrix.
//
//    vec4 toWorldSpace = inverse(Matrices.projectionMatrix) * vec4(TexCoords.xy, 1.0f, 1.0f);
//    vec3 viewDir = normalize(vec3(TexCoords.xy * 2 - 1, 1.0));
//
//
//    // Compute the starting point of the ray (camera position)
//    vec3 rayOrigin = Matrices.viewPos;
//
//    // Determine the maximum distance to integrate (e.g., atmosphere radius)
//    float atmosphereRadius = 10.0; // Adjust based on your scene's scale
//    float tMax = atmosphereRadius;
//
//    // Step size for integration
//    float dt = tMax / float(NUM_SAMPLES);
//
//    // Accumulators for scattering
//    vec3 totalRayleigh = vec3(0.0);
//    vec3 totalMie = vec3(0.0);
//
//    // Optical depth accumulators
//    float opticalDepthR = 0.0;
//    float opticalDepthM = 0.0;
//
//    // Integration loop along the view ray
//    for (int i = 0; i < NUM_SAMPLES; i++) {
//        float t = dt * (float(i) + 0.5);
//        vec3 samplePosition = rayOrigin + viewDir * t;
//
//        // Calculate height (assuming ground is at y=0)
//        float height = length(samplePosition) - 0.0; // Adjust if your ground is elsewhere
//
//        // Compute local density based on exponential atmosphere
//        float densityR = exp(-height / HR);
//        float densityM = exp(-height / HM);
//
//        // Accumulate optical depth
//        opticalDepthR += densityR * dt;
//        opticalDepthM += densityM * dt;
//
//        // Calculate scattering from the sun at the sample position
//        float mu = dot(viewDir, sunDirection);
//        float phaseR = rayleighPhase(mu);
//        float phaseM = miePhase(mu);
//
//        // Accumulate scattering (weight by density)
//        totalRayleigh += densityR * dt * phaseR;
//        totalMie += densityM * dt * phaseM;
//    }
//
//    // Combine scattering contributions
//    vec3 scattering = betaR * totalRayleigh + betaM * totalMie;
//    
//    return scattering * 100000;
//}
