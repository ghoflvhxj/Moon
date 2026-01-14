#include "PSCommon.hlsli"

PixelOut_CombinePass main(PixelIn pIn)
{
    PixelOut_CombinePass pOut;

    float4 diffuse	= G_Diffuse.Sample(g_Sampler, pIn.uv);
	float4 DirectionalLight	= G_LightDiffuse.Sample(g_Sampler, pIn.uv);
	float4 specular = g_LightSpecular.Sample(g_Sampler, pIn.uv);
    float4 PointLight = G_PointLightDiffuse.Sample(g_Sampler, pIn.uv);
    float4 Emissive = G_Emissive.Sample(g_Sampler, pIn.uv);
    
    float TexelWidth = 1.f / resolution.x;
    float TexelHeight = 1.f / resolution.y;
    
    float4 Blur = float4(0.f, 0.f, 0.f, 0.f);
    int Count = 25;
    int A = Count / 2;
    for (int i = -A; i <= A; ++i)
    {
        for (int j = -A; j <= A; ++j)
        {
            Blur += G_Emissive.Sample(g_Sampler, pIn.uv + float2(i * TexelWidth, j * TexelHeight));
        }
    }
    Blur /= pow(Count, 2);
    
    pOut.color = diffuse;
    
    if (bLight)
    {
        pOut.color = diffuse * (DirectionalLight + PointLight + specular) + (Emissive + Blur);
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

    return pOut;
}