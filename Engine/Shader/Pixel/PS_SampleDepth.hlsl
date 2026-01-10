#include "../../PSCommon.hlsli"

cbuffer PS_Depth : register(b3)
{
    float DepthZ = 0.f;
};

float4 main(PixelIn_SimpleTex pIn) : SV_TARGET0
{
    float Depth = g_ShadowDepth.Sample(g_Sampler, float3(pIn.uv, 0.f));
    return float4(Depth, Depth, Depth, 1.f);
}