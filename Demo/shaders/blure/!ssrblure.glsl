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

layout (std140, binding = 1) uniform Standard2
{
    int SCR_WIDTH;
	int SCR_HEIGHT;
};


uniform sampler2D ssr;

void main()
{
	vec3 color = vec3(0.0);
    
    // Sample a 3x3 box around the current pixel
    for (int x = -2; x <= 2; ++x) {
        for (int y = -2; y <= 2; ++y) {
            vec2 offset = vec2(x, y) * 1.0 / vec2(textureSize(ssr, 0));
            color += texture(ssr, TexCoords + offset).rgb;
        }
    }
    
    // Divide by the total number of samples (3x3 = 9)
    color /= 25.0;
    
    FragColor = vec4(color, 1.0);
}
#endif
