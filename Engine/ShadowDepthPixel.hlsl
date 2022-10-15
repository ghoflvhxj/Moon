#include "PSCommon.hlsli"

PixelOut_ShadowDepth main(PixelIn pIn)
{
    PixelOut_ShadowDepth pOut;
    pOut.shadowDepth = pIn.pos.z / pIn.pos.w;
    //pOut.shadowDepth = float4(1.f, 1.f, 1.f, 1.f);
    
    return pOut;
}