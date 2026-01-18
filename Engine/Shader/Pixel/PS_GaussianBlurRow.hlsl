#include "../../PSCommon.hlsli"

StructuredBuffer<float> Weights : register(t100);

float4 main(PixelIn pIn) : SV_TARGET
{
    return float4(GaussianBlur(T_Diffuse, pIn.uv, Weights, true), 1.0f);
}