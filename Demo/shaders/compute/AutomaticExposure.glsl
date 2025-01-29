#version 450

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout(rgba16f, binding = 0) uniform image2D imgInput;

layout(std430, binding = 1) buffer LuminanceBuffer 
{
    double logLum;
};

layout (std140, binding = 1) uniform Standard2
{
    int SCR_WIDTH;
	int SCR_HEIGHT;
	int time;

	float sunSize;
	vec3 sunPosition;
	vec3 sunColor;
	float padding;
	float sunStrength;
	float Luminance;
};


void main()
{
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec3 hdrColor = imageLoad(imgInput, texelCoord).rgb;
    float luminance = dot(hdrColor, vec3(0.2125, 0.7154, 0.0721));
    
//	if(luminance >= 1.0f)
//	{
//		return;
//	}

    logLum += log(luminance + 0.00001);
    if((texelCoord.x == (SCR_WIDTH - 1 )) && (texelCoord.y == (SCR_HEIGHT - 1)))
	{
		logLum = exp(float(logLum / (SCR_WIDTH * SCR_HEIGHT)));
		return;
	} 
}