
static const float2 aPos[6] =
{
    float2(-1.0f, 1.0f),
    float2(-1.0f, -1.0f),
    float2(1.0f, -1.0f),
    float2(-1.0f, 1.0f),
    float2(1.0f, -1.0f),
    float2(1.0f, 1.0f)
};

static const float2 aTexCoords[6] =
{
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(1.0f, 1.0f)
};

[shader("vertex")]
float2 vertex_Sun(uint vertexId: SV_VulkanVertexID, out float4 position: SV_Position)
{
    //position = Camera.projectionMatrix * mat4(mat3(Camera.viewMatrix)) * Atmosphere.sunModelMatrix * vec4(aPos[vertexId] * Atmosphere.sunSize * 0.1, 0.0f, 1.0f);

    position = float4(0.0f);
    return aTexCoords[vertexId];
}

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