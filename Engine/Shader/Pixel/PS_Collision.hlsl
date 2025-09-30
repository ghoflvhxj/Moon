#include "../../PSCommon.hlsli"

PixelOut_Simple main(PixelIn_Simple PixelIn)
{
    PixelOut_Simple PixelOut = (PixelOut_Simple)0;
    PixelOut.color = PixelIn.color;
    //PixelOut.color = float4(1.f, 0.f, 0.f, 1.f);

    return PixelOut;
}