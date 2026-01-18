#include "../../PSCommon.hlsli"

StructuredBuffer<float> Weights : register(RENDERPASS_STURCTURED_BUFFER);

float4 main(PixelIn pIn) : SV_TARGET
{
    return float4(GaussianBlur(T_Diffuse, pIn.uv, Weights, false), 1.0f);
}