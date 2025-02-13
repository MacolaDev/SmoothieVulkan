#version 450 core

//Vertex shader
#ifdef VERTEX_SHADER

vec3 aPos[36] = vec3[](
            vec3(-1.0f, -1.0f, -1.0f),  // bottom-left
            vec3(1.0f,  1.0f, -1.0f),  // top-right
            vec3(1.0f, -1.0f, -1.0f),  // bottom-right         
            vec3(1.0f,  1.0f, -1.0f),  // top-right
            vec3(-1.0f, -1.0f, -1.0f),  // bottom-left
            vec3(-1.0f,  1.0f, -1.0f),  // top-left
            // front face
            vec3(-1.0f, -1.0f,  1.0f),  // bottom-left
            vec3( 1.0f, -1.0f,  1.0f),  // bottom-right
            vec3( 1.0f,  1.0f,  1.0f),  // top-right
            vec3( 1.0f,  1.0f,  1.0f),  // top-right
            vec3(-1.0f,  1.0f,  1.0f),  // top-left
            vec3(-1.0f, -1.0f,  1.0f),  // bottom-left
            // left face
            vec3(-1.0f,  1.0f,  1.0f),  // top-right
            vec3(-1.0f,  1.0f, -1.0f),  // top-left
            vec3(-1.0f, -1.0f, -1.0f),  // bottom-left
            vec3(-1.0f, -1.0f, -1.0f),  // bottom-left
            vec3(-1.0f, -1.0f,  1.0f),  // bottom-right
            vec3(-1.0f,  1.0f,  1.0f),  // top-right
            // right face
            vec3(1.0f,  1.0f,  1.0f),   // top-left
            vec3(1.0f, -1.0f, -1.0f),   // bottom-right
            vec3(1.0f,  1.0f, -1.0f),   // top-right         
            vec3(1.0f, -1.0f, -1.0f),   // bottom-right
            vec3(1.0f,  1.0f,  1.0f),   // top-left
            vec3(1.0f, -1.0f,  1.0f),   // bottom-left     
            // bottom face
            vec3(-1.0f, -1.0f, -1.0f),   // top-right
            vec3( 1.0f, -1.0f, -1.0f),   // top-left
            vec3( 1.0f, -1.0f,  1.0f),   // bottom-left
            vec3( 1.0f, -1.0f,  1.0f),   // bottom-left
            vec3(-1.0f, -1.0f,  1.0f),   // bottom-right
            vec3(-1.0f, -1.0f, -1.0f),   // top-right
            // top face
            vec3(-1.0f,  1.0f, -1.0f),   // top-left
            vec3( 1.0f,  1.0f , 1.0f),   // bottom-right
            vec3( 1.0f,  1.0f, -1.0f),   // top-right     
            vec3( 1.0f,  1.0f,  1.0f),   // bottom-right
            vec3(-1.0f,  1.0f, -1.0f),   // top-left
            vec3(-1.0f,  1.0f,  1.0f)   // bottom-left   
);

layout (set = 0, binding = 0) uniform Standard
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
	vec3 viewPos;
	mat4 projectionViewMatrix;
};

layout(location = 0) out vec3 TexCoords;
void main()
{
	TexCoords = aPos[gl_VertexIndex];
	mat4 rotView = mat4(mat3(viewMatrix));
	vec4 position = projectionMatrix * rotView * vec4(aPos[gl_VertexIndex], 1.0);
	gl_Position = position.xyww;
}

#endif 

//Fragment shader
#ifdef FRAGMENT_SHADER

layout(location = 0) in vec3 TexCoords;
layout(location = 0) out vec4 FragColor;

layout(set = 2, binding = 0)uniform samplerCube environmentMap;
void main()
{
	vec3 image = texture(environmentMap, TexCoords).rgb;
	FragColor = vec4(image, 1);
}
#endif 