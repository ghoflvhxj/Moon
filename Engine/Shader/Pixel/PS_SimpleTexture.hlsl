#include "../../PSCommon.hlsli"

float4 main(PixelIn_SimpleTex pIn) : SV_TARGET0
{
    float3 Color = T_Diffuse.Sample(g_Sampler, pIn.uv).xyz;
    
    clip(Color - float3(0.01f, 0.01f, 0.01f) * bAlphaMask);
    clip(Color - 0.001f * bAlphaMask);
    
    return float4(Color, 1.f);
}