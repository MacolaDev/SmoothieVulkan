#include "../include/Smoothie.slang"
#include "../include/lighting_functions.slang"

ParameterBlock<Smoothie_Globals> g_SmoothieGlobals;

struct Lighting_Global
{
    TextureCube irradiance_map;
    TextureCube prefilter_map;
    Texture2D brdf_LUT;
    
}

static const float MAX_REFLECTION_LOD = 4.0;
ParameterBlock<Lighting_Global> g_LightingGlobal;   

[shader("fragment")]
float4 global_illumination(float2 TexCoords)
{
    var _attachments = g_SmoothieGlobals.attachments;
    var _samplers = g_SmoothieGlobals.samplers;
    var _camera = g_SmoothieGlobals.camera;

    var _gbuff_samp = _samplers.Texture2DSampler;

    float3 albedo = _attachments.gAlbedo.Sample(_gbuff_samp, TexCoords).rgb;
    albedo = pow(albedo, float3(2.2));
    
    float3 _mrao = _attachments.gMRAO.Sample(_gbuff_samp, TexCoords).rgb;
    float metallic = _mrao.r;
    float roughness = _mrao.g;
    float ao = _mrao.b;
    
    float3 FragPos = _attachments.gPosition.Sample(_gbuff_samp, TexCoords).xyz;

    // input lighting data
    float3 N = normalize(_attachments.gNormal.Sample(_gbuff_samp, TexCoords).xyz);
    float3 V = normalize(_camera.viewPos - FragPos);
    float3 R = normalize(reflect(-V, N)); 

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    float3 F0 = float3(0.04); 
    F0 = lerp(F0, albedo, metallic);

    float3 Lo = float3(0.0f);
    
    // ambient lighting (we now use IBL as the ambient term)
    float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    float3 irradiance = g_LightingGlobal.irradiance_map.Sample(_gbuff_samp, N).rgb;

    float3 diffuse = irradiance * albedo;

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    float _brdf_sample_lod = roughness * MAX_REFLECTION_LOD;
    float3 prefilteredColor = g_LightingGlobal.prefilter_map.SampleLevel(_gbuff_samp, R, _brdf_sample_lod).rgb;
    float2 brdf = g_LightingGlobal.brdf_LUT.Sample(_gbuff_samp, float2(max(dot(N, V), 0.0), roughness)).rg;
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    float3 ambient = (kD * diffuse + specular) * ao;

    return float4(ambient, 1.0f);
}

//******************************************** BRDF precompute ********************************************//

[shader("fragment")]
float2 brdf_precompute(float2 TexCoords)
{
    float2 integratedBRDF = IntegrateBRDF(TexCoords.x, TexCoords.y);
    return integratedBRDF;
}
