#include "../../PSCommon.hlsli"

struct TestPixelIn
{
    float4 pos : SV_POSITION;
    uint renderTargetIndex : SV_RenderTargetArrayIndex;
    float4 Distance : POSITION0;
};

struct PixelOut_PointShadowDepth
{
    float ShadowDepth : SV_TARGET0;
};

PixelOut_PointShadowDepth main(TestPixelIn pIn)
{
    PixelOut_PointShadowDepth pOut = (PixelOut_PointShadowDepth)0;
    pOut.ShadowDepth = pIn.Distance.z;
	
    return pOut;
}