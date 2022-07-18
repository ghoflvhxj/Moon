#include "PSCommon.hlsli"

Texture2D g_Albedo		: register(t0);
Texture2D g_Light		: register(t1);
Texture2D g_Specular	: register(t2);

SamplerState g_Sampler;

struct PixelOut
{
	float4	color	: SV_TARGET0;
};

PixelOut main(PixelIn pIn)
{
	PixelOut pOut;

	float4 diffuse	= g_Albedo.Sample(g_Sampler, pIn.uv);
	float4 light	= g_Light.Sample(g_Sampler, pIn.uv);
	float4 specular = g_Specular.Sample(g_Sampler, pIn.uv);
	
	//pOut.color = diffuse;
	pOut.color = diffuse * (light + specular);
	//pOut.color = light;

	return pOut;
}