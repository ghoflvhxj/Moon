#include "PSCommon.hlsli"

struct PixelOut_PointShadowDepth
{
    float4 shadowDepth : SV_TARGET0;
};

PixelOut_PointShadowDepth main(PixelIn pIn)
{
    PixelOut_PointShadowDepth pOut;
    
    pOut.shadowDepth = pIn.pos.z;
    
    return pOut;
}