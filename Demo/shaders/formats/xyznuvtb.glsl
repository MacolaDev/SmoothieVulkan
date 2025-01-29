#version 450 core
layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec4 aNormals;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 aTangents;
layout (location = 4) in vec4 aBitangents;

layout (std140, binding = 0) uniform Matrices
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
};

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

vec3 unpackNormals(vec4 packedNormal)
{   
    float x = packedNormal.x/127 - 1;
    float y = packedNormal.y/127 - 1;
    float z = packedNormal.z/127 - 1;
    return vec3(x, y, z); 
}

out vec2 TexCoord;
out vec3 vertexNormals;
out vec4 FragPos;
out mat3 TBN;


void main() 
{   
    
    TexCoord = aTexCoord;
    FragPos = modelMatrix * vec4(aPos, 1);
    //mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
    vertexNormals =  normalize(-normalMatrix * vec3(unpackNormals(aNormals)));

    //T, B and N should be multipled with normal matrix 
    vec3 T = normalize(vec3(modelMatrix * vec4(normalMatrix * unpackNormals(aTangents), 0.0)));
    vec3 B = normalize(vec3(modelMatrix * vec4(normalMatrix * unpackNormals(aBitangents), 0.0)));
    vec3 N = normalize(vec3(modelMatrix * vec4(normalMatrix * unpackNormals(aNormals), 0.0)));
    TBN = mat3(T, B, N); 
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos, 1.0); 
}