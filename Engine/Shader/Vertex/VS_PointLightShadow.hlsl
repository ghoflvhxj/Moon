#include "../../VSCommon.hlsli"

struct VertexOut_PointShadowDepth
{
    float4 Pos : SV_Position;
    uint InstanceID : SV_InstanceID;
    float Distance : DISTANCE;
    uint PointLightIndex : POINTLIGHTINDEX;
};

cbuffer CBuffer_RenderPass : register(CBUFFER_RENDERPASS)
{
    float4 LightPos;
    int PointLightIndex;
};

cbuffer RenderPassObject : register(CBUFFER_RENDERPASSOBJECT)
{
    row_major matrix Transforms[6]; // Object World * Light View * Light Proj
}

VertexOut_PointShadowDepth main(VertexIn vIn, InstanceIn iIn)
{
    VertexOut_PointShadowDepth vOut = (VertexOut_PointShadowDepth) 0;
    
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
    vOut.Distance = length(LightPos.xyz - mul(animatedPos, worldMatrix).xyz);
    vOut.PointLightIndex = PointLightIndex;
    
    return vOut;
}