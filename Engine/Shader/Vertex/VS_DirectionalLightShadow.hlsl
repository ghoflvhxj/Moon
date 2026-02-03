#include "../../VSCommon.hlsli"

//cbuffer Test : register(CBUFFER_RENDERPASS)
//{
//    float4 LightPos;
//    int PointLightIndex;
//};

cbuffer RenderPassObject : register(CBUFFER_RENDERPASSOBJECT)
{
    row_major matrix Transforms[4]; // Object World * Light View * Light Proj
}

VertexOut_ShadowDepth main(VertexIn vIn, InstanceIn iIn)
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
    
    //vOut.Pos = mul(animatedPos, worldMatrix);
    
    vOut.Pos = mul(animatedPos, Transforms[iIn.InstanceID]);
    vOut.InstanceID = iIn.InstanceID;
    
    return vOut;
}