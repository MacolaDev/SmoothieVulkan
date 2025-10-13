#version 450 core


#ifdef VERTEX_SHADER
layout (location = 0) in vec3 aPos;
layout (std140, binding = 0) uniform Standard
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
	vec3 viewPos;
    mat4 projectionViewMatrix;
};
uniform mat4 modelMatrix;
void main()
{
	gl_Position = projectionViewMatrix * modelMatrix * vec4(aPos, 1.0f);
}
#endif

#ifdef FRAGMENT_SHADER
out vec4 FragColor;
void main()
{
	FragColor = vec4(0, 1.0f, 0, 1.0f);
}
#endif
