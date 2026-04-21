
static const float2 aPos_QUAD[6] = 
{
    float2(-1.0f, 1.0f),
    float2(-1.0f, -1.0f),
    float2(1.0f, -1.0f),
    float2(-1.0f, 1.0f),
    float2(1.0f, -1.0f),
    float2(1.0f, 1.0f)
};

static const float2 aTexCoords_QUAD[6] = 
{
	float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 0.0f),
	float2(1.0f, 1.0f)
};

[shader("vertex")]
float2 vertex_QUAD(int vertexIndex: SV_VulkanVertexID, out float4 position: SV_Position)
{
    position = float4(aPos_QUAD[vertexIndex].x, aPos_QUAD[vertexIndex].y, 0.0, 1.0);
    return aTexCoords_QUAD[vertexIndex];
}
