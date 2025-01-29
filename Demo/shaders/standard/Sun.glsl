#version 450

layout (std140, binding = 0) uniform Standard
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
	vec3 viewPos;
    mat4 projectionViewMatrix;
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


#ifdef VERTEX_SHADER
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;


out vec2 TexCoords;
uniform mat4 modelMatrix;

void main()
{	
	gl_Position = projectionMatrix * mat4(mat3(viewMatrix)) * modelMatrix * vec4(aPos * sunSize * 0.1, 0.0f, 1.0f);
	
	TexCoords = aTexCoords;
}
#endif

#ifdef FRAGMENT_SHADER
out vec4 FragColor;
in vec2 TexCoords;
void main()
{
	vec2 uv = TexCoords * 2 - 1;
	float DISTANCE = 1.0f - length(uv);
	gl_FragDepth = 0.99999f;

	if(DISTANCE > 0.0f)
	{
		FragColor = vec4(sunColor * sunStrength, 1.0f);
	}
	else
	{
		discard;
	}
}
#endif