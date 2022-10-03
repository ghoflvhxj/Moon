#include "VSCommon.hlsli"

VertexOut main(VertexIn vIn)
{
    VertexOut vOut;
    matrix worldViewProj = mul(mul(worldMatrix, viewMatrixForShadow), orthographicForShadow[0]);

    if (animated == false)
    {
        vOut.pos = mul(float4(vIn.pos, 1.f), worldViewProj);
    }
    else
    {
        matrix boneTransform = keyFrameMatrices[vIn.blendIndex[0]];
        float4 animatedPos = mul(float4(vIn.pos, 1.f), boneTransform);

        vOut.pos = mul(float4(animatedPos.x, animatedPos.y, animatedPos.z, 1.f), worldViewProj);
    }

    return vOut;
}