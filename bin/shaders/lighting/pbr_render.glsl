#version 450 core

//Fragment shader
#ifdef SSHADER_FRAGMENT_GLOBAL_ILLUMINATION


#extension GL_ARB_shading_language_include: enable 
#include "../ShadersGeneral.glslh"

#define DESCRIPTOR_CAMERA MAKE_DESCRIPTOR_SET(0, 0)
#define DESCRIPTOR_GENERAL MAKE_DESCRIPTOR_SET(1, 0)
//#define DESCRIPTOR_ATMOSPHERE MAKE_DESCRIPTOR_SET(3, 0)
#include "../Descriptors.glslh"


layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 FragColor;

layout (set = 2, binding = 0) uniform sampler2D gPosition;
layout (set = 2, binding = 1) uniform sampler2D gNormal;
layout (set = 2, binding = 2) uniform sampler2D gAlbedo;
layout (set = 2, binding = 3) uniform sampler2D gMRAO;
layout (set = 2, binding = 4) uniform sampler2D gDepth;

layout (set = 3, binding = 0) uniform sampler2D brdfLUT;

layout (set = 4, binding = 0) uniform samplerCube irradianceMap;
layout (set = 4, binding = 1) uniform samplerCube prefilterMap;

//layout (set = 2, binding = 8) uniform sampler2D dynamicSSA0;
//layout (set = 2, binding = 8) uniform sampler2D SSR;


const float PI = 3.14159265359;


// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   

// vec3 sunLo(vec3 N, vec3 V, float roughness, vec3 F0, float metallic, vec3 albedo)
// {
//     vec3 sunPos = vec3(Atmosphere.sunWorldPosition.x, Atmosphere.sunWorldPosition.z, -Atmosphere.sunWorldPosition.y);
//     vec3 L = normalize(-sunPos); //Sun is directional light
//     vec3 H = normalize(V + L);
//     float NdotL = max(dot(N, L), 0.0);

//     vec3 radiance = Atmosphere.sunColor * Atmosphere.sunStrength * NdotL;

//     // Cook-Torrance BRDF
//     float NDF = DistributionGGX(N, H, roughness);   
//     float G   = GeometrySmith(N, V, L, roughness);    
//     vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
    
//     vec3 numerator    = NDF * G * F;
//     float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
//     vec3 specular = numerator / denominator;
    

//     vec3 kS = F;
//     vec3 kD = vec3(1.0) - kS;
//     kD *= 1.0 - metallic;	                

//     return (kD * albedo / PI + specular) * radiance;
// }

const float MAX_REFLECTION_LOD = 4.0;


const float gFogEnd = 100.0f;
const float gLayeredFogTop = 10.5f;
const vec3 forColor = vec3(0.2f);

float CalcLayeredFogFactor(vec3 FragPos, vec3 viewPos)
{
    vec3 CameraProj = viewPos;
    CameraProj.y = 0.0;

    vec3 PixelProj = FragPos;
    PixelProj.y = 0.0;

    float DeltaD = length(CameraProj - PixelProj) / gFogEnd;

    float DeltaY = 0.0f;
    float DensityIntegral = 0.0f;

    if (viewPos.y > gLayeredFogTop) 
    { 
        // The camera is above the top of the fog
        if (FragPos.y < gLayeredFogTop) 
        {
            // The pixel is inside the fog
            DeltaY = (gLayeredFogTop - FragPos.y) / gLayeredFogTop;
            DensityIntegral = DeltaY * DeltaY * 0.5;
        }                                     // else - the pixel is above the fog - nothing to do
    } else { // The camera is inside the fog
        if (FragPos.y < gLayeredFogTop)
        {   
            // The pixel is inside the fog
            DeltaY = abs(viewPos.y - FragPos.y) / gLayeredFogTop;
            float DeltaCamera = (gLayeredFogTop - viewPos.y) / gLayeredFogTop;
            float DensityIntegralCamera = DeltaCamera * DeltaCamera * 0.5;
            float DeltaPixel = (gLayeredFogTop - FragPos.y) / gLayeredFogTop;
            float DensityIntegralPixel = DeltaPixel * DeltaPixel * 0.5;
            DensityIntegral = abs(DensityIntegralCamera - DensityIntegralPixel);
        } else 
        {   
            // The pixel is above the fog
            DeltaY = (gLayeredFogTop - viewPos.y) / gLayeredFogTop;
            DensityIntegral = DeltaY * DeltaY * 0.5;
        }
    }

    float FogDensity = 0.0;

    if (DeltaY != 0) 
    {
        FogDensity = (sqrt(1.0 + ((DeltaD / DeltaY) * (DeltaD / DeltaY)))) * DensityIntegral;
    }

    float FogFactor = exp(-FogDensity);

    return FogFactor;
}


void main()
{
	
	vec3 albedo = pow(texture(gAlbedo, TexCoords).rgb, vec3(2.2));
	float metallic = texture(gMRAO, TexCoords).r;
	float roughness = texture(gMRAO, TexCoords).g;

	float ao = texture(gMRAO, TexCoords).b; //* texture(dynamicSSA0, TexCoords).r;

    vec3 FragPos = texture(gPosition, TexCoords).xyz;
	
    //vec3 SSRColor = texture(SSR, TexCoords).rgb;

    // input lighting data
    vec3 N = normalize(texture(gNormal, TexCoords).xyz);
    vec3 V = normalize(Camera.viewPos - FragPos);
    vec3 R = normalize(reflect(-V, N)); 

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    //Sun light
    vec3 Lo = vec3(0.0f);//sunLo(N, V, roughness, F0, metallic, albedo);
    
    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(irradianceMap, N).rgb;

    vec3 diffuse = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo;// + SSRColor;

    //color = mix(color, forColor, 1 - CalcLayeredFogFactor(FragPos, Camera.viewPos));

    FragColor = vec4(color, 1.0f);
}




#endif
