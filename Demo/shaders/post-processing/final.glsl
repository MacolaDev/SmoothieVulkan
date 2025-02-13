#version 450 core

//Fragment shader
#ifdef FRAGMENT_SHADER

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

const mat3x3 ACESInputMat = mat3x3
(
	0.59719, 0.35458, 0.04823,
	0.07600, 0.90834, 0.01566,
	0.02840, 0.13383, 0.83777
);

const mat3x3 ACESOutputMat = mat3x3
(
	1.60475,  -0.53108, -0.07367,
   -0.10208,   1.10813, -0.00605,
   -0.00327,  -0.07276,  1.07602
);

vec3 RRTAndODTFit(vec3 v)
{
	vec3 a = v * (v + 0.0245786f) - 0.000090537f;
	vec3 b = v * (0.9783729f * v + 0.4329510f) + 0.238081f;
	return a / b;
}

vec3 ASECFittet(vec3 color)
{
	color = transpose(ACESInputMat) * color;
	color = RRTAndODTFit(color);
	color = transpose(ACESOutputMat) * color;
	color = clamp(color, 0, 1);
	return color;
}


layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 FragColor;

layout(set = 2, binding = 0) uniform sampler2D HDRImage;
layout(set = 2, binding = 1) uniform sampler2D BloomImage;

const float luminance = 0.9f;
const float exposure = 0.3f;
const float bloomIntensity = 0.1f;

vec2 flipUV(vec2 uv)
{
	return vec2(uv.x, 1.0 - uv.y);
}

void main()
{
	vec3 final = vec3(0.0f, 0.0f, 0.0f);
	vec3 hdrImage = texture(HDRImage, flipUV(TexCoords)).rgb;
	vec3 bloom = texture(BloomImage, flipUV(TexCoords)).rgb;

	final += bloom * bloomIntensity;
	final += hdrImage;
	
	final *= luminance * exposure;
	
	//Tone mapping
	final = ASECFittet(final);
	
	//gamma correction
	final = pow(final, vec3(1/2.2f));

	FragColor = vec4(final, 1.0f);
}
#endif 