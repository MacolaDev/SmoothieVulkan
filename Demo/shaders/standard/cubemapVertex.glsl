#version 450

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

layout (location = 0) out vec3 TexCoords;
layout(push_constant, std430) uniform pc
{
    mat4 projection;
	mat4 view;
} Matrices;

void main()
{
	TexCoords = aPos[gl_VertexIndex];
	gl_Position = Matrices.projection * Matrices.view * vec4(aPos[gl_VertexIndex], 1.0);
}
#endif