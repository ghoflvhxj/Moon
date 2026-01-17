#include "../../PSCommon.hlsli"

float4 main(PixelIn pIn) : SV_TARGET
{
    return float4(GaussianBlur(T_Diffuse, pIn.uv, true), 1.0f);
}