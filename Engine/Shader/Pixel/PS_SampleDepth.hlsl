#include "../../PSCommon.hlsli"

cbuffer PS_Depth : register(b3)
{
    float DepthZ = 0.f;
};

float4 main(PixelIn_SimpleTex pIn) : SV_TARGET0
{
    float CascadeIndex = 0.f;
    float Depth = g_ShadowDepth.Sample(g_Sampler, float3(pIn.uv, CascadeIndex));
    return float4(Depth, Depth, Depth, 0.f);
}