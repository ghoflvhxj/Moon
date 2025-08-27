#include "../../PSCommon.hlsli"

PixelOut_CombinePass main(PixelIn pIn)
{
    PixelOut_CombinePass pOut = (PixelOut_CombinePass)0;
    pOut.color = float4(1.f, 1.f, 1.f, 1.f);
    return pOut;
}