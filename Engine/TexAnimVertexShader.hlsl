#include "VSCommon.hlsli"

VertexOut main(VertexIn vIn)
{
	VertexOut vOut;

	matrix worldView = mul(worldMatrix, viewMatrix);
	matrix worldViewProj = mul(worldView, projectionMatrix);

	matrix boneTransform =
	{
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f
	};

	for (int i = 0; i < 4; ++i)
	{
		boneTransform += mul(keyFrameMatrices[vIn.blendIndex[i]], vIn.blendWeight[i]);
	}

	float4 animatedPos = mul(float4(vIn.pos, 1.f), boneTransform);
	vOut.pos = mul(float4(animatedPos.x, animatedPos.y, animatedPos.z, 1.f), worldViewProj);
	vOut.uv = vIn.uv;

	matrix lightView = mul(worldMatrix, viewMatrixForShadow);
	matrix lightViewProj = mul(lightView, orthographicForShadow[0]);
    vOut.shadowPos = mul(float4(animatedPos.x, animatedPos.y, animatedPos.z, 1.f), lightViewProj);
    vOut.shadowPos.x = (vOut.shadowPos.x + 1.f) / 2.f;
    vOut.shadowPos.y = (vOut.shadowPos.y - 1.f) / -2.f;
    vOut.shadowUV = float2(vOut.shadowPos.x, vOut.shadowPos.y);

	vOut.normal = mul(float4(vIn.normal, 0.f), worldView).xyz;
	vOut.tangent = mul(float4(vIn.tangent, 0.f), worldView).xyz;
	vOut.binormal = mul(float4(vIn.binormal, 0.f), worldView).xyz;

	return vOut;
}
