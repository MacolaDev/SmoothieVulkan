#version 450 core

#extension GL_ARB_shading_language_include: enable 
#include "../ShadersGeneral.glslh"

#define DESCRIPTOR_CAMERA MAKE_DESCRIPTOR_SET(0, 0)
#include "../Descriptors.glslh"
#ifdef SSHADER_VERTEX_SKYBOX_VERTEX

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

layout(location = 0) out vec3 TexCoords;
void main()
{
	TexCoords = aPos[gl_VertexIndex];
	mat4 rotView = mat4(mat3(Camera.viewMatrix));
	vec4 position = Camera.projectionMatrix * rotView * vec4(aPos[gl_VertexIndex], 1.0);
	gl_Position = position.xyww;
}

#endif 

//Fragment shader
#ifdef SSHADER_FRAGMENT_SKYBOX_FRAGMENT

layout(location = 0) in vec3 TexCoords;
layout(location = 0) out vec4 FragColor;

layout(set = 2, binding = 0) uniform samplerCube environmentMap;

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


void main()
{
	vec3 image = texture(environmentMap, TexCoords).rgb;
	FragColor = vec4(image, 1);
}
#endif 