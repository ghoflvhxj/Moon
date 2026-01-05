#include "../../PSCommon.hlsli"

float GetStencil(float2 InUV, int2 InOffset)
{
    int2 UV = { InUV.x * resolution.x, InUV.y * resolution.y };
    return T_Stencil.Load(int3(UV, 0), int2(InOffset)).g;
}

PixelOut_CombinePass main(PixelIn pIn)
{
    PixelOut_CombinePass pOut = (PixelOut_CombinePass)0;
    
    float Stencil = GetStencil(pIn.uv, int2(0, 0));
    clip(Stencil == 0.f);
    
    float2 Texel = float2(1.f / resolution.x, 1.f / resolution.y);
    float Left = GetStencil(pIn.uv, int2(-1, 0));
    float Right = GetStencil(pIn.uv, int2(1, 0));
    float Top = GetStencil(pIn.uv, int2(0, -1));
    float Bottom = GetStencil(pIn.uv, int2(0, 1));

    bool bEdge = (Left != Stencil) || (Right != Stencil) || (Top != Stencil) || (Bottom != Stencil);

    pOut.color.rgb = bEdge ? float3(1.f, 1.f, 1.f) : float3(0.f, 0.f, 0.f);

    return pOut;
}