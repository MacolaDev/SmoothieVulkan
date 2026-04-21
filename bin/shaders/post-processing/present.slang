
//layout (std140, binding = 1) uniform Standard2
//{
//    int SCR_WIDTH;
//	int SCR_HEIGHT;
//	int time;
//
//	float sunSize;
//	vec3 sunPosition;
//	vec3 sunColor;
//	float padding;
//	float sunStrength;
//	float Luminance;
//};

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

float2 flipUV(float2 uv)
{
    return float2(uv.x, 1.0 - uv.y);
}

static const float luminance = 0.9f;
static const float exposure = 0.3f;
static const float bloomIntensity = 0.1f;

[vk_binding(0, 2)] Sampler2D HDRImage;
[vk_binding(1, 2)] Sampler2D BloomImage;

[shader("fragment")]
float4 fragment_PRESENT(float2 TexCoords)
{
	float3 final = float3(0.0f, 0.0f, 0.0f);
	float3 hdrImage = HDRImage.Sample(flipUV(TexCoords)).rgb;
	float3 bloom = BloomImage.Sample(flipUV(TexCoords)).rgb;

	final += hdrImage;
	final += bloom * bloomIntensity;
	
	// vec3 godRays = texture(godRaysImage, flipUV(TexCoords)).rgb;
	// final += godRays;

	final *= luminance * exposure;
	
	//Tone mapping
	final = ASECFittet(final);
	
	//gamma correction
	final = pow(final, float3(1/2.2f));

	return float4(final, 1.0f);
}