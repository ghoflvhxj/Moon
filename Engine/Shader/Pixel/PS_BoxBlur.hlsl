#include "../../PSCommon.hlsli"

float4 main(PixelIn pIn) : SV_TARGET0
{
    return float4(BoxBlur(T_Diffuse, pIn.uv, int2(5, 5)), 1.0f);
}