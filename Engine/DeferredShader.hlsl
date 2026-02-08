#include "PSCommon.hlsli"

PixelOut_CombinePass main(PixelIn pIn)
{
    PixelOut_CombinePass pOut;

    float4 Albedo	= G_Diffuse.Sample(g_Sampler, pIn.uv);

    float4 DirectDiffuse = G_LightDirectDiffuse.Sample(g_Sampler, pIn.uv);
    float4 DirectSpecular = G_LightDirectSpecular.Sample(g_Sampler, pIn.uv);
    float4 InDirectDiffuse = G_IndirectDiffuse.Sample(g_Sampler, pIn.uv);
    
    float4 Emissive = G_Emissive.Sample(g_Sampler, pIn.uv);
    float4 EmissiveBlur = G_EmissiveUpSampled.Sample(g_Sampler, pIn.uv);
    float4 AO = bSSAO ? G_SSAOUpSample.Sample(g_Sampler, pIn.uv) : 1.f;
    pOut.color = Albedo;
    
    if (bLight)
    {
        pOut.color = (Albedo * DirectDiffuse + DirectSpecular) + (Emissive + EmissiveBlur) + (Albedo * InDirectDiffuse * AO);
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
    //pOut.color = Albedo;
    
    if (bDebugDirectLight)
    {
        pOut.color = DirectDiffuse + DirectSpecular;
    }
    
    if (bDebugInDirectLight)
    {
        pOut.color = InDirectDiffuse * AO;
    }
    
    if (bDebugSSAO)
    {
        pOut.color = AO;
    }

    return pOut;
}