#include "../../PSCommon.hlsli"

struct TestPixelIn
{
    float4 pos : SV_POSITION;
    uint renderTargetIndex : SV_RenderTargetArrayIndex;
    float Distance : DISTANCE;
};

struct PixelOut_PointShadowDepth
{
    float ShadowDepth : SV_TARGET0;
};

PixelOut_PointShadowDepth main(TestPixelIn pIn)
{
    PixelOut_PointShadowDepth pOut = (PixelOut_PointShadowDepth)0;
    pOut.ShadowDepth = pIn.Distance;
	
    return pOut;
}