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
        boneTransform = (matrix) 0;
        for (int i = 0; i < 4; ++i)
        {
            if (vIn.blendIndex[i] != -1)
            {
                boneTransform += mul(keyFrameMatrices[vIn.blendIndex[i]], vIn.blendWeight[i]);
            }
        }
    }
	
	float3 animatedPos = mul(float4(vIn.pos.xyz, 1.f), boneTransform);
    vOut.pos = Temp(vIn);
    vOut.worldPos = mul(float4(animatedPos, 1.f), worldMatrix).xyz;
	vOut.uv			= vIn.uv;
    vOut.Clip		= vOut.pos.zw;

    float4x4 SkinnedWorldView = mul(boneTransform, WorldView);
    vOut.normal = mul(float4(vIn.normal, 0.f), SkinnedWorldView).xyz;
    vOut.tangent = mul(float4(vIn.tangent, 0.f), SkinnedWorldView).xyz;
    vOut.binormal = mul(float4(vIn.binormal, 0.f), SkinnedWorldView).xyz;

	return vOut;
}
