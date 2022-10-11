#include "VSCommon.hlsli"

VertexOut main(VertexIn vIn)
{
    VertexOut vOut = (VertexOut)0;

    if (animated == false)
    {
        vOut.pos = mul(float4(vIn.pos, 1.f), worldMatrix);
    }
    else
    {
        matrix boneTransform = keyFrameMatrices[vIn.blendIndex[0]];
        float4 animatedPos = mul(float4(vIn.pos, 1.f), boneTransform);

        vOut.pos = mul(float4(animatedPos.x, animatedPos.y, animatedPos.z, 1.f), worldMatrix);
    }
 
    return vOut;
}