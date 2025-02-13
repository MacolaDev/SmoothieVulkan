#version 450

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 0) uniform sampler2D HDRITexture;

void main()
{
	vec3 color = texture(HDRITexture, TexCoords).rgb;
	if(length(color) > 1.0f)
	{
		FragColor = vec4(color, 1.0f);
		return;
	}
	FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
#endif
