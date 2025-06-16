#include "VSCommon.hlsli"

VertexOut main(VertexIn vIn)
{
    VertexOut vOut = (VertexOut)0;
    
    matrix boneTransform =
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
    
    float4 animatedPos = mul(float4(vIn.pos, 1.f), boneTransform);
    animatedPos.w = 1.f;
    
    vOut.pos = mul(animatedPos, worldMatrix);
    vOut.worldPos = mul(animatedPos, worldMatrix).xyz;
    
    return vOut;
}