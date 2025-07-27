#include "../../PSCommon.hlsli"

float4 main(PixelIn_SimpleTex pIn) : SV_TARGET0
{
    return float4(g_Diffuse.Sample(g_Sampler, pIn.uv).xyz, 1.f);
}