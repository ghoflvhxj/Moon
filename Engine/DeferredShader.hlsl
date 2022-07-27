#include "PSCommon.hlsli"

struct PixelOut
{
	float4	color	: SV_TARGET0;
};

PixelOut main(PixelIn pIn)
{
	PixelOut pOut;

	float4 diffuse	= g_Diffuse.Sample(g_Sampler, pIn.uv);

	float2 lightUV  = pIn.uv;
	lightUV.x += -1.f;
	lightUV.x *= -1.f;
	float4 light	= g_LightDiffuse.Sample(g_Sampler, lightUV);
	float4 specular = g_LightSpecular.Sample(g_Sampler, pIn.uv);
	
	pOut.color = diffuse;
	pOut.color = diffuse * (light + specular);
	//pOut.color = light;

	return pOut;
}