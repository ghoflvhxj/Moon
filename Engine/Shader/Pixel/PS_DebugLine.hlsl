#include "../../PSCommon.hlsli"

PixelOut_Simple main(PixelIn_Simple PixelIn)
{
    PixelOut_Simple PixelOut = (PixelOut_Simple)0;
    PixelOut.color = float4(1.f, 1.f, 1.f, 1.f);
    return PixelOut;
}