#include "PSCommon.hlsli"

struct PixelOut
{
	float4	color	: SV_TARGET0;
};

PixelOut_CombinePass main(PixelIn pIn)
{
    PixelOut_CombinePass pOut;

	float4 diffuse	= g_Diffuse.Sample(g_Sampler, pIn.uv);
	float4 DirectionalLightDiffuse	= g_LightDiffuse.Sample(g_Sampler, pIn.uv);
	float4 specular = g_LightSpecular.Sample(g_Sampler, pIn.uv);
    float4 PointLightDiffuse = T_PointLightDiffuse.Sample(g_Sampler, pIn.uv);
	pOut.color = diffuse;
	if (bLight)
	{
        pOut.color = diffuse * (DirectionalLightDiffuse + PointLightDiffuse + specular);
    }
    //pOut.color = light;
    
    pOut.color += T_Collision.Sample(g_Sampler, pIn.uv);

	return pOut;
}