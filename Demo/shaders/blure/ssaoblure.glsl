#version 450 core

//Vertex shader
#ifdef VERTEX_SHADER
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
out vec2 TexCoords;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
#endif

#ifdef FRAGMENT_SHADER
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D ssao;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(ssao, 0));
	float result = 0.0;
	for (int x = -2; x < 2; ++x)
	{
		for (int y = -2; y < 2; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(ssao, TexCoords + offset).r;
		}
	}
	FragColor = vec4(vec3(result / (4.0 * 4.0)), 1.0f);
}
#endif
