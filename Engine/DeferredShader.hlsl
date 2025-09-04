#include "PSCommon.hlsli"

PixelOut_CombinePass main(PixelIn pIn)
{
    PixelOut_CombinePass pOut;

	float4 diffuse	= g_Diffuse.Sample(g_Sampler, pIn.uv);
	float4 DirectionalLight	= g_LightDiffuse.Sample(g_Sampler, pIn.uv);
	float4 specular = g_LightSpecular.Sample(g_Sampler, pIn.uv);
    float4 PointLight = T_PointLightDiffuse.Sample(g_Sampler, pIn.uv);
	pOut.color = diffuse;
	if (bLight)
	{
        pOut.color = diffuse * (DirectionalLight + PointLight + specular);
    }
    
    float4 Collision = T_Collision.Sample(g_Sampler, pIn.uv);
    if (Collision.x > 0.f)
    {
        pOut.color = Collision;
    }

    float4 Outline = T_Outline.Sample(g_Sampler, pIn.uv);
    if (Outline.x > 0.f)
    {
        pOut.color = Outline;
    }

	return pOut;
}