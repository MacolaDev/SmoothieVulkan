#version 450

#extension GL_ARB_shading_language_include: enable 
#include "../ShadersGeneral.glslh"

#define DESCRIPTOR_CAMERA MAKE_DESCRIPTOR_SET(0, 0)
#define DESCRIPTOR_GENERAL MAKE_DESCRIPTOR_SET(1, 0)
#define DESCRIPTOR_ATMOSPHERE MAKE_DESCRIPTOR_SET(2, 0)
#include "../Descriptors.glslh"

#ifdef SSHADER_FRAGMENT_GODRAYS

layout (set = 3, binding = 0) uniform sampler2D depthTexture;

const int NUM_STEPS = 50;
const float DENSITY = 0.05;  // Strength of effect
const float DECAY = 0.95;    // Fade over distance
const float WEIGHT = 0.3;    // Brightness
const float EXPOSURE = 0.5;  // Overall brightness control
const vec3 raysColor = vec3(1.0, 0.9, 0.7);

vec3 ViewPosFromDepth(vec2 uv, float depth) 
{
    vec4 clipSpacePos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 worldPos = Camera.invProjectionMatrix * clipSpacePos;
    return worldPos.xyz / worldPos.w;
}

layout (location = 0) in vec2 TexCoords;
layout (location = 0) out vec4 FragColor;

vec2 ToScreenSpace(vec3 worldSpace)
{
    worldSpace = vec3(worldSpace.x, worldSpace.z, -worldSpace.y);
    vec4 position = Camera.projectionMatrix * mat4(mat3(Camera.viewMatrix)) * vec4(worldSpace, 1.0f);
    return (position.xy/position.w) * 0.5f + 0.5f;
}

float hash(vec2 p) 
{
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() 
{
    

    float sceneDepth = texture(depthTexture, TexCoords).r;

    vec3 fragPos = ViewPosFromDepth(TexCoords, sceneDepth);
    vec3 uvViewPos = ViewPosFromDepth(TexCoords, 0.0f);


    vec3 sunViewPosition = vec3(Camera.viewMatrix * vec4(Atmosphere.sunWorldPosition, 1.0f));
    
    vec3 rayDirWorld = normalize(Camera.viewPos - Atmosphere.sunWorldPosition);
    vec3 rayDirView = vec3(Camera.viewMatrix * vec4(rayDirWorld, 1.0f));

    float stepSize = 3.0f;
    float drop = 0.1;

    vec2 sunScreenSpace = ToScreenSpace(Atmosphere.sunWorldPosition);

    // Compute the vector from center to the current fragment
    vec2 dir = TexCoords - sunScreenSpace;
    
    FragColor = vec4(vec3(0.0f), 1.0f);

//    float distanceToSun = length(TexCoords - sunScreenSpace.xy);
//    float shadow = 0.0f;
//    float illumination = clamp((1 - distanceToSun), 0.0f, 1.0f);
//    
//    for (int i = 0; i < NUM_STEPS; i++) 
//    {
//        vec3 rayMove = uvViewPos + rayDirView * (i * stepSize / NUM_STEPS);
//        vec4 screenPos = Camera.projectionMatrix * vec4(rayMove, 1.0f);
//        screenPos.xyz /= screenPos.w;
//        if(screenPos.z > sceneDepth) 
//        {   
//            shadow = smoothstep(0.0f, 1.0f, shadow / (NUM_STEPS * drop));
//            FragColor = vec4(vec3(shadow * illumination) * raysColor, 1.0f);
//            return;
//        }
//        shadow +=  drop;
//    }
//
//    //No hit, return color
//    FragColor = vec4(vec3(illumination) * raysColor, 1.0f);
}

#endif
