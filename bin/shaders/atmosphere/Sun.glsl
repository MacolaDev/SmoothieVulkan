#version 450
#extension GL_ARB_shading_language_include: enable 
#include "../ShadersGeneral.glslh"

#define DESCRIPTOR_CAMERA MAKE_DESCRIPTOR_SET(0, 0)
#define DESCRIPTOR_GENERAL MAKE_DESCRIPTOR_SET(1, 0)
#define DESCRIPTOR_ATMOSPHERE MAKE_DESCRIPTOR_SET(2, 0)
#include "../Descriptors.glslh"
//********* VERTEX SHADER ***********//
#ifdef SSHADER_VERTEX_SUN_VERTEX
vec2 aPos[6] = vec2[](
	vec2(-1.0f, 1.0f),
	vec2(-1.0f, -1.0f),
	vec2(1.0f, -1.0f),
	vec2(-1.0f, 1.0f),
	vec2(1.0f, -1.0f),
	vec2(1.0f, 1.0f)
);
vec2 aTexCoords[6] = vec2[](

    vec2(0.0f, 1.0f),
    vec2(0.0f, 0.0f),
    vec2(1.0f, 0.0f),
    vec2(0.0f, 1.0f),
    vec2(1.0f, 0.0f),
    vec2(1.0f, 1.0f)
);

layout (location = 0) out vec2 TexCoords;

void main()
{	
	gl_Position = Camera.projectionMatrix * mat4(mat3(Camera.viewMatrix)) * Atmosphere.sunModelMatrix * vec4(aPos[gl_VertexIndex] * Atmosphere.sunSize * 0.1, 0.0f, 1.0f);
	
	TexCoords = aTexCoords[gl_VertexIndex];
}
#endif

//************ FRAMGNET *********//
#ifdef SSHADER_FRAGMENT_SUN_FRAGMENT
layout (location = 0) out vec4 FragColor;
layout (location = 0) in vec2 TexCoords;

void main()
{
	vec2 uv = TexCoords * 2 - 1;
	float DISTANCE = 1.0f - length(uv);
	gl_FragDepth = 1.0f;

	if(DISTANCE > 0.0f)
	{
		FragColor = vec4(Atmosphere.sunColor * Atmosphere.sunStrength, 1.0f);
	}
	else
	{
		discard;
	}
}
#endif