Texture2D g_Diffuse;

SamplerState g_Sampler;

cbuffer CBuffer
{
	bool bUseNormalTexture;
};

struct VertexOut
{
	float4 pos		: SV_POSITION;
	float2 uv		: TEXCOORD0;
};

float4 main(VertexOut vOut) : SV_TARGET
{
	float4 color = g_Diffuse.Sample(g_Sampler, vOut.uv);

	return color;
}