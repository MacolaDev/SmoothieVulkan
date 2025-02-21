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
	mat4 view;
} Matrices;

const float pi = 3.141592;

mat4 getProjectionMatrix(float fovy, float aspec, float zNear, float zFar)
{
    mat4 matrix = mat4(0.0f);
    matrix[0][0] = 1.0f;
    matrix[1][1] = 1.0f;
    matrix[2][2] = 1.0f;
    matrix[3][3] = 1.0f;

    float toRadians = fovy*pi/180;
	float f = 1/tan(toRadians/2);
	matrix[0][0] = f / aspec;
	matrix[1][1] = f;
	float value = zNear - zFar;

	matrix[2][2] = (zFar + zNear) / value;

	matrix[3][2] = (2 * zFar * zNear) / value;
	matrix[2][3] = -1.0f;
	matrix[3][3] = 0.0f;
    return matrix;

}

void main()
{
	TexCoords = aPos[gl_VertexIndex];
	mat4 projectionMatrix = getProjectionMatrix(90.0f, 1.0f, 0.1f, 10.0f);
    gl_Position = projectionMatrix * Matrices.view * vec4(aPos[gl_VertexIndex], 1.0);
}
#endif