#include "PSCommon.hlsli"

PixelOut_ShadowDepth main(PixelIn pIn)
{
    PixelOut_ShadowDepth pOut;
    pOut.shadowDepth = pIn.Clip.x; // ���������̹Ƿ� z���� ���
    //pOut.shadowDepth = float4(pIn.pos.z, 0.f, 0.f, 1.f);
    //clip(pOut.shadowDepth > 0.f ? 1.f : -1.f);
    
    return pOut;
}