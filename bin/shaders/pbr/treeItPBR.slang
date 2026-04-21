#include "../Smoothie.slang"

struct VertexShaderInput
{
    float3 aPos;
    uint aNormals;
    float2 aTexCoord;
    uint aTangents;
    uint aBitangents;
    uint aColor;
}

struct VertexShaderOutput
{
    float2 TexCoord;
    float4 FragPos;
	float materialID;
	float3x3 TBN;
}

[vk_binding(0, 2)]
cbuffer ModelData
{
    float4x4 modelMatrix;
    uint ModelID;

    Sampler2D diffuseTexture;
    Sampler2D roughnessTexture;
    Sampler2D normalTexture;
}

static const float4 TreeWindSize = float4(1000, 400, 100, 10);
static const float4 TreeWindTimer = float4(1, 1, 1, 1) * 0.01;
static const float3 TreeWindPower = float3(0.3, 0.1, 0); //wind effect on leafs, brances, bark

[TargetPipeline("standard")]
[shader("vertex")]
VertexShaderOutput mainVertex(VertexShaderInput in, out float4 position: SV_Position)
{
    VertexShaderOutput out;
    out.TexCoord = in.aTexCoord;
	
	float4 normals = unpackNormals(in.aNormals).xyzw;
	float4 color = unpackNormals(in.aColor).xyzw;

	float3 T = normalize(mul(modelMatrix, unpackNormals(in.aTangents)).xyz);
	float3 N = normalize(mul(modelMatrix, unpackNormals(in.aNormals)).xyz);
	T = normalize(T - dot(T, N) * N);
	float3 B = cross(N, T);
    out.TBN = float3x3(T, B, N); 
	
    float4 WorldPos = mul(modelMatrix, float4(in.aPos, 1.0f));

    float time = 0.0f;

    float3 GlobalWind = (WorldPos.xyz / TreeWindSize.x) + TreeWindTimer.x * time;
	GlobalWind = float3(cos(GlobalWind.x),0,sin(GlobalWind.z));
	GlobalWind.y = 1 + abs(dot(GlobalWind.x,GlobalWind.z));

    float3 WindForce = (WorldPos.xyz / TreeWindSize.x) + TreeWindTimer.y * time - GlobalWind;
	WindForce = float3(cos(WindForce.x), 0,sin(WindForce.y));

    float3 WindAnimate = WindForce * TreeWindPower.x * (WorldPos.y / TreeWindSize.y);
    float3 WindVariant = float3(TreeWindTimer.z * time + (color.y/255.0 * 10));
	WindAnimate.xz += float2(cos(WindVariant.x), sin(WindVariant.z)) * TreeWindPower.y * color.z/255.0;
	
	float3 WindFlutter = (WorldPos.xyz / TreeWindSize.w)+(TreeWindTimer.w * time)+color.y/255.0;
	WindAnimate += float3(abs(normalize(mul(modelMatrix, normals)).xyz) * sin(dot(WindFlutter, float3(1.0f))) * TreeWindPower.z * (color.x/255.0 - 0.5));
	WorldPos += float4(WindAnimate * GlobalWind.y, 0.0f);

	out.FragPos = WorldPos;
	position = mul(Camera.projectionViewMatrix, WorldPos);

    out.materialID = normals.w;
    return out;
}

[TargetPipeline("standard")]
[shader("fragment")]
gBufferPass mainFragment(VertexShaderOutput in)
{
	float2 UV = float2(in.TexCoord.x/3.0f + in.materialID/3.0f, in.TexCoord.y);

    float4 color = diffuseTexture.Sample(UV);
	float roughness = roughnessTexture.Sample(UV).x;
	float3 normal = normalTexture.Sample(UV).xyz;

	if(color.w < 0.5f) discard;

    normal = normalize(normal * 2 - 1);
	
    gBufferPass out;
	out.gAlbedo = float4(color.rgb, 1.0f);
    out.gNormal = float4(abs(normalize(mul(in.TBN, normal))), 1);
    out.gMRAO = float4(0.0f, roughness, 1.0f, 1.0f);
    out.gPosition = in.FragPos;
    return out;
}