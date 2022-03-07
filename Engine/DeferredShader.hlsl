Texture2D g_Albedo		: register(t0);
Texture2D g_Light		: register(t1);
Texture2D g_Specular	: register(t2);

SamplerState g_Sampler;

struct PixelIn
{
	float4 pos		: SV_POSITION;
	float2 uv		: TEXCOORD0;
	float3 normal	: NORMAL0;
	float3 tangent	: NORMAL1;
	float3 binormal : NORMAL2;
};

struct PixelOut
{
	float4	color	: SV_TARGET0;
};

cbuffer Dummy
{
	float4 a;
};

PixelOut main(PixelIn pIn)
{
	PixelOut pOut;

	float4 diffuse	= g_Albedo.Sample(g_Sampler, pIn.uv);
	float4 light	= g_Light.Sample(g_Sampler, pIn.uv);
	float4 specular = g_Specular.Sample(g_Sampler, pIn.uv);
	
	//pOut.color = diffuse * light;
	pOut.color = diffuse * (light + specular);

	return pOut;
}