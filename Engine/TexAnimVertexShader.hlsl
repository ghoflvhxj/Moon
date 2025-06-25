#include "VSCommon.hlsli"

VertexOut main(VertexIn vIn)
{
	VertexOut vOut;

	matrix worldView = mul(worldMatrix, viewMatrix);
	matrix worldViewProj = mul(worldView, projectionMatrix);

	matrix boneTransform =
	{
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};

    if (animated)
    {
        boneTransform = (matrix)0;
        for (int i = 0; i < 4; ++i)
        {
            boneTransform += mul(keyFrameMatrices[vIn.blendIndex[i]], vIn.blendWeight[i]);
        }
    }
	
	float3 animatedPos = mul(float4(vIn.pos.xyz, 1.f), boneTransform);
    vOut.pos = mul(float4(animatedPos, 1.f), worldViewProj);
    vOut.worldPos = mul(float4(animatedPos, 1.f), worldMatrix).xyz;
	vOut.uv			= vIn.uv;
    vOut.Clip		= vOut.pos.zw;
    
    //vOut.normal = normalize(mul(float4(vIn.normal, 0.f), boneTransform).xyz);
    //vOut.tangent = normalize(mul(float4(vIn.tangent, 0.f), boneTransform).xyz);
    //vOut.binormal = normalize(mul(float4(vIn.binormal, 0.f), boneTransform).xyz);
    
    float4x4 SkinnedWorldView = mul(boneTransform, worldView);
    vOut.normal = mul(float4(vIn.normal, 0.f), SkinnedWorldView).xyz;
    vOut.tangent = mul(float4(vIn.tangent, 0.f), SkinnedWorldView).xyz;
    vOut.binormal = mul(float4(vIn.binormal, 0.f), SkinnedWorldView).xyz;
    
    //vOut.normal = normalize(mul(float4(vIn.normal, 0.f), worldMatrix).xyz);
    //vOut.tangent = normalize(mul(float4(vIn.tangent, 0.f), worldMatrix).xyz);
    //vOut.binormal = normalize(mul(float4(vIn.binormal, 0.f), worldMatrix).xyz);
    
    //vOut.normal = normalize(mul(float4(vIn.normal, 0.f), worldView).xyz);
    //vOut.tangent = normalize(mul(float4(vIn.tangent, 0.f), worldView).xyz);
    //vOut.binormal = normalize(mul(float4(vIn.binormal, 0.f), worldView).xyz);
    
    //vOut.normal = vIn.normal;
    //vOut.tangent = vIn.tangent;
    //vOut.binormal = vIn.binormal;
	return vOut;
}
