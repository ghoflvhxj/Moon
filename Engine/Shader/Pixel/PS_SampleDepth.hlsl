#include "../../PSCommon.hlsli"

float4 main(PixelIn_SimpleTex pIn) : SV_TARGET0
{
    float CascadeIndex = 0.f;
    float Depth = G_ShadowDepth.Sample(g_Sampler, float3(pIn.uv, CascadeIndex));
    return float4(Depth, Depth, Depth, 0.f);
}