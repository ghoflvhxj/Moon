#include "VSCommon.hlsli"

VertexOut main(VertexIn vIn)
{
	VertexOut vOut;

	matrix worldView = mul(worldMatrix, viewMatrix);
	matrix worldViewProj = mul(worldView, projectionMatrix);

	matrix boneTransform = keyFrameMatrices[vIn.blendIndex[0]];
	float4 animatedPos = mul(float4(vIn.pos, 1.f), boneTransform);

	vOut.pos = mul(float4(animatedPos.x, animatedPos.y, animatedPos.z, 1.f), worldViewProj);
	vOut.uv = vIn.uv;
	vOut.normal = mul(float4(vIn.normal, 0.f), worldView).xyz;
	vOut.tangent = mul(float4(vIn.tangent, 0.f), worldView).xyz;
	vOut.binormal = mul(float4(vIn.binormal, 0.f), worldView).xyz;

	return vOut;
}
