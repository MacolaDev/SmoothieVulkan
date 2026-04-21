// TODO: Make this non-constant
static const float invGamma = 1.0 / 2.2;

float3 PowVec3(float3 v, float p)
{
    return float3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

float3 ToSRGB(float3 v)   
{ 
	return PowVec3(v, invGamma); 
}

float sRGBToLuma(float3 col)
{
    //return dot(col, vec3(0.2126f, 0.7152f, 0.0722f));
	return dot(col, float3(0.299f, 0.587f, 0.114f));
}

float KarisAverage(float3 col)
{
	// Formula is 1 / (1 + luma)
	float luma = sRGBToLuma(ToSRGB(col)) * 0.25f;
	return 1.0f / (1.0f + luma);
}

[vk_push_constant]
cbuffer DOWNSAMPLE_PC
{
    float2 srcResolution;
    int mipLevel;
}

[vk_push_constant]
cbuffer UPSAMPLE_PC
{
    float filterRadius;
};

[vk_binding(0, 0)]
Sampler2D srcTexture;

[shader("fragment")]
float4 fragment_HIGLIGHTS(float2 TexCoords)
{
    float3 color = srcTexture.Sample(TexCoords).rgb;
	if(length(color) > 1.0f)
	{
        return float4(color, 1.0f);
	}
	return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

[shader("fragment")]
float3 fragment_DOWNSAMPLE(float2 TexCoords)
{
    float3 downsample = float3(0.0f);
	float2 srcTexelSize = 1.0 / srcResolution;
	float x = srcTexelSize.x;
	float y = srcTexelSize.y;

	// Take 13 samples around current texel:
	// a - b - c
	// - j - k -
	// d - e - f
	// - l - m -
	// g - h - i
	// === ('e' is the current texel) ===
	float3 a = srcTexture.Sample(float2(TexCoords.x - 2*x, TexCoords.y + 2*y)).rgb;
	float3 b = srcTexture.Sample(float2(TexCoords.x,       TexCoords.y + 2*y)).rgb;
	float3 c = srcTexture.Sample(float2(TexCoords.x + 2*x, TexCoords.y + 2*y)).rgb;
									  
	float3 d = srcTexture.Sample(float2(TexCoords.x - 2*x, TexCoords.y)).rgb;
	float3 e = srcTexture.Sample(float2(TexCoords.x,       TexCoords.y)).rgb;
	float3 f = srcTexture.Sample(float2(TexCoords.x + 2*x, TexCoords.y)).rgb;
									  
	float3 g = srcTexture.Sample(float2(TexCoords.x - 2*x, TexCoords.y - 2*y)).rgb;
	float3 h = srcTexture.Sample(float2(TexCoords.x,       TexCoords.y - 2*y)).rgb;
	float3 i = srcTexture.Sample(float2(TexCoords.x + 2*x, TexCoords.y - 2*y)).rgb;
									  
	float3 j = srcTexture.Sample(float2(TexCoords.x - x, TexCoords.y + y)).rgb;
	float3 k = srcTexture.Sample(float2(TexCoords.x + x, TexCoords.y + y)).rgb;
	float3 l = srcTexture.Sample(float2(TexCoords.x - x, TexCoords.y - y)).rgb;
	float3 m = srcTexture.Sample(float2(TexCoords.x + x, TexCoords.y - y)).rgb;

	// Apply weighted distribution:
	// 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
	// a,b,d,e * 0.125
	// b,c,e,f * 0.125
	// d,e,g,h * 0.125
	// e,f,h,i * 0.125
	// j,k,l,m * 0.5
	// This shows 5 square areas that are being sampled. But some of them overlap,
	// so to have an energy preserving downsample we need to make some adjustments.
	// The weights are the distributed, so that the sum of j,k,l,m (e.g.)
	// contribute 0.5 to the final color output. The code below is written
	// to effectively yield this sum. We get:
	// 0.125*5 + 0.03125*4 + 0.0625*4 = 1

	// Check if we need to perform Karis average on each block of 4 samples
	float3 groups[5];
	switch (mipLevel)
	{
	case 0:
	  // We are writing to mip 0, so we need to apply Karis average to each block
	  // of 4 samples to prevent fireflies (very bright subpixels, leads to pulsating
	  // artifacts).
	  groups[0] = (a+b+d+e) * (0.125f/4.0f);
	  groups[1] = (b+c+e+f) * (0.125f/4.0f);
	  groups[2] = (d+e+g+h) * (0.125f/4.0f);
	  groups[3] = (e+f+h+i) * (0.125f/4.0f);
	  groups[4] = (j+k+l+m) * (0.5f/4.0f);
	  groups[0] *= KarisAverage(groups[0]);
	  groups[1] *= KarisAverage(groups[1]);
	  groups[2] *= KarisAverage(groups[2]);
	  groups[3] *= KarisAverage(groups[3]);
	  groups[4] *= KarisAverage(groups[4]);
	  downsample = groups[0]+groups[1]+groups[2]+groups[3]+groups[4];
	  downsample = max(downsample, 0.0001f);
	  break;
	default:
	  downsample = e*0.125;                // ok
	  downsample += (a+c+g+i)*0.03125;     // ok
	  downsample += (b+d+f+h)*0.0625;      // ok
	  downsample += (j+k+l+m)*0.125;       // ok
	  break;
	}

    return downsample;
}

[shader("fragment")]
float3 fragment_UPSAMPLE(float2 TexCoords)
{

    float3 upsample = float3(0.0f);
	// The filter kernel is applied with a radius, specified in texture
	// coordinates, so that the radius will vary across mip resolutions.
	float x = filterRadius;
	float y = filterRadius;

	// Take 9 samples around current texel:
	// a - b - c
	// d - e - f
	// g - h - i
	// === ('e' is the current texel) ===
	float3 a = srcTexture.Sample(float2(TexCoords.x - x, TexCoords.y + y)).rgb;
	float3 b = srcTexture.Sample(float2(TexCoords.x,     TexCoords.y + y)).rgb;
	float3 c = srcTexture.Sample(float2(TexCoords.x + x, TexCoords.y + y)).rgb;

	float3 d = srcTexture.Sample(float2(TexCoords.x - x, TexCoords.y)).rgb;
	float3 e = srcTexture.Sample(float2(TexCoords.x,     TexCoords.y)).rgb;
	float3 f = srcTexture.Sample(float2(TexCoords.x + x, TexCoords.y)).rgb;

	float3 g = srcTexture.Sample(float2(TexCoords.x - x, TexCoords.y - y)).rgb;
	float3 h = srcTexture.Sample(float2(TexCoords.x,     TexCoords.y - y)).rgb;
	float3 i = srcTexture.Sample(float2(TexCoords.x + x, TexCoords.y - y)).rgb;

	// Apply weighted distribution, by using a 3x3 tent filter:
	//  1   | 1 2 1 |
	// -- * | 2 4 2 |
	// 16   | 1 2 1 |
	upsample = e*4.0;
	upsample += (b+d+f+h)*2.0;
	upsample += (a+c+g+i);
    upsample *= 1.0 / 16.0;
    return upsample;
}