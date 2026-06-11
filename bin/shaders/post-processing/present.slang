#include "../Smoothie.slang"


static const float3x3 ACESInputMat = float3x3
(
	0.59719, 0.35458, 0.04823,
	0.07600, 0.90834, 0.01566,
	0.02840, 0.13383, 0.83777
);

static const float3x3 ACESOutputMat = float3x3
(
	1.60475,  -0.53108, -0.07367,
   -0.10208,   1.10813, -0.00605,
   -0.00327,  -0.07276,  1.07602
);

float3 RRTAndODTFit(float3 v)
{
	float3 a = v * (v + 0.0245786f) - 0.000090537f;
	float3 b = v * (0.9783729f * v + 0.4329510f) + 0.238081f;
	return a / b;
}

float3 ASECFittet(float3 color)
{
	color = mul(ACESInputMat, color);
	color = RRTAndODTFit(color);
	color = mul(ACESOutputMat, color);
	color = clamp(color, 0, 1);
	return color;
}


static const float luminance = 0.9f;
static const float exposure = 0.3f;
static const float bloomIntensity = 0.1f;

struct PresentData
{
    //Texture2D Bloom_Image;
}

ParameterBlock<Smoothie_Globals> g_GlobalData;
ParameterBlock<PresentData> g_PresentData;

[shader("fragment")]
float4 fragment_PRESENT(float2 TexCoords)
{

    var _samplers = g_GlobalData.samplers;
    var _attachments = g_GlobalData.attachments;
    
    float2 _uv = float2(TexCoords.x, 1.0 - TexCoords.y);
    float3 final = float3(0.0f, 0.0f, 0.0f);

    float3 hdrImage = _attachments.HDR.Sample(_samplers.Texture2DSampler, _uv).rgb;
    final += hdrImage;

	//float3 bloom = g_PresentData.Bloom_Image.Sample(_samplers.Texture2DSampler, _uv).rgb;
	//final += bloom * bloomIntensity;
	
	// vec3 godRays = texture(godRaysImage, flipUV(TexCoords)).rgb;
	// final += godRays;

	final *= luminance * exposure;
	
	//Tone mapping
	final = ASECFittet(final);
	
	//gamma correction
	final = pow(final, float3(1/2.2f));

	return float4(final, 1.0f);
}

static const float2 aPos_QUAD[6] =
    {
        float2(-1.0f, 1.0f),
        float2(-1.0f, -1.0f),
        float2(1.0f, -1.0f),
        float2(-1.0f, 1.0f),
        float2(1.0f, -1.0f),
        float2(1.0f, 1.0f)
    };

static const float2 aTexCoords_QUAD[6] =
    {
        float2(0.0f, 1.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(0.0f, 1.0f),
        float2(1.0f, 0.0f),
        float2(1.0f, 1.0f)
    };

[shader("vertex")]
float2 vertex_QUAD(int vertexIndex: SV_VulkanVertexID, out float4 position: SV_Position)
{
    position = float4(aPos_QUAD[vertexIndex].x, aPos_QUAD[vertexIndex].y, 0.0, 1.0);
    return aTexCoords_QUAD[vertexIndex];
}
