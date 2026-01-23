#include "PSCommon.hlsli"

PixelOut_CombinePass main(PixelIn pIn)
{
    PixelOut_CombinePass pOut;

    float4 diffuse	= G_Diffuse.Sample(g_Sampler, pIn.uv);
	float4 DirectionalLight	= G_LightDiffuse.Sample(g_Sampler, pIn.uv);
	float4 specular = g_LightSpecular.Sample(g_Sampler, pIn.uv);
    float4 PointLight = G_PointLightDiffuse.Sample(g_Sampler, pIn.uv);
    float4 Emissive = G_Emissive.Sample(g_Sampler, pIn.uv);
    float4 EmissiveBlur = G_EmissiveUpSampled.Sample(g_Sampler, pIn.uv);
    
    pOut.color = diffuse;
    
    if (bLight)
    {
        pOut.color = (diffuse + specular) * (DirectionalLight + PointLight) + (Emissive + EmissiveBlur);
    }
    
    float4 Collision = G_Collision.Sample(g_Sampler, pIn.uv);
    if (any(Collision.rgb != 0.f))
    {
        pOut.color = Collision;
    }

    float4 Outline = G_Outline.Sample(g_Sampler, pIn.uv);
    if (any(Outline.rgb != 0.f))
    {
        pOut.color = Outline;
    }
    
    /******************
        디버깅 용
    ******************/

    //pOut.color = diffuse;
    
    if (bDebugDirectionalLight)
    {
        pOut.color = DirectionalLight;
    }
    
    if (bDebugPointLight)
    {
        pOut.color = PointLight;
    }

    return pOut;
}