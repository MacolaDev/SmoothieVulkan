//Standard vertex shader for post-processing effects (draws a quad on a screen)

#ifdef VERTEX_SHADER

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
	gl_Position = vec4(aPos[gl_VertexIndex].x, aPos[gl_VertexIndex].y, 0.0, 1.0);
	TexCoords = aTexCoords[gl_VertexIndex];
}
#endif