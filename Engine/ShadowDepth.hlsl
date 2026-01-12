#include "VSCommon.hlsli"

VertexOut_ShadowDepth main(VertexIn vIn)
{
    VertexOut_ShadowDepth vOut = (VertexOut_ShadowDepth) 0;
    
    row_major matrix boneTransform =
    {
        1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
    };

    if (animated)
    {
        boneTransform *= 0.f;
        for (int i = 0; i < 4; ++i)
        {
            boneTransform += mul(keyFrameMatrices[vIn.blendIndex[i]], vIn.blendWeight[i]);
        }
    }
    
    float4 animatedPos = mul(float4(vIn.pos.xyz, 1.f), boneTransform);
    animatedPos.w = 1.f;
    
    vOut.Pos = mul(animatedPos, worldMatrix);
    
    return vOut;
}