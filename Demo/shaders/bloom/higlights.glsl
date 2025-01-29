#version 450

#ifdef FRAGMENT_SHADER
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D HDRITexture;

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

//Vertex shader
#ifdef VERTEX_SHADER
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
out vec2 TexCoords;

void main()
{
	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
	TexCoords = aTexCoords;
}
#endif