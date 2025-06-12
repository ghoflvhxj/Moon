#include "PSCommon.hlsli"

float4 main(PixelIn_Simple pIn) : SV_TARGET0
{
    return float4(pIn.color.xyz, 1.f);
}