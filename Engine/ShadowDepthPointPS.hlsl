#include "PSCommon.hlsli"

struct PixelOut_PointShadowDepth
{
    float4 shadowDepth : SV_TARGET7;
};

PixelOut_PointShadowDepth main(PixelIn pIn)
{
    PixelOut_PointShadowDepth pOut;
    pOut.shadowDepth = pIn.Clip.x; // 직교투영이므로 z값을 사용
    //pOut.shadowDepth = float4(pIn.pos.z, 0.f, 0.f, 1.f);
    //clip(pOut.shadowDepth > 0.f ? 1.f : -1.f);
    
    return pOut;
}