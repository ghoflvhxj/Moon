#include "PSCommon.hlsli"

struct PixelOut_PointShadowDepth
{
    float4 shadowDepth : SV_TARGET0;
};

PixelOut_PointShadowDepth main(PixelIn pIn)
{
    PixelOut_PointShadowDepth pOut;
    
    // 포인트 라이트와 픽셀의 월드 거리를 저장함
    pOut.shadowDepth = float4(pIn.Clip.x, pIn.Clip.x, pIn.Clip.x, pIn.Clip.x);
    
    return pOut;
}