#include "../../PSCommon.hlsli"

PixelOut_CombinePass main(PixelIn pIn)
{
    PixelOut_CombinePass pOut = (PixelOut_CombinePass)0;
    
    float2 Texel = float2(1.f / resolution.x, 1.f / resolution.y);

    float Color = T_Stencil.Sample(g_Sampler, pIn.uv).r;
    float Left = T_Stencil.Sample(g_Sampler, pIn.uv - Texel.x).r;
    float Right = T_Stencil.Sample(g_Sampler, pIn.uv + Texel.x).r;
    float Top = T_Stencil.Sample(g_Sampler, pIn.uv - Texel.y).r;
    float Bottom = T_Stencil.Sample(g_Sampler, pIn.uv + Texel.y).r;

    bool bEdge = (Left != Color) || (Right != Color) || (Top != Color) || (Bottom != Color);

    pOut.color.rgb = bEdge ? float3(1.f, 1.f, 1.f) : float3(0.f, 0.f, 0.f);

    return pOut;
}