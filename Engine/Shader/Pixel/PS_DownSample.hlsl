#include "../../PSCommon.hlsli"

float4 main(PixelIn pIn) : SV_TARGET0
{   
    return float4(T_Diffuse.Sample(S_Linear, pIn.uv).xyz, 1.0f);
}