#include "VSCommon.hlsli"

VertexOut main(VertexIn vIn)
{
	VertexOut vOut;

	matrix worldView = mul(worldMatrix, viewMatrix);
	matrix worldViewProj = mul(worldView, projectionMatrix);

	matrix lightView = mul(worldMatrix, viewMatrixForShadow);
	matrix lightViewProj = mul(lightView, orthographicForShadow[0]);
	float4 temp = mul(float4(vIn.pos.x, vIn.pos.y, vIn.pos.z, 1.f), lightViewProj);
	temp.x = (temp.x + 1.f) / 2.f;
	temp.y = (temp.y - 1.f) / -2.f;

	vOut.pos		= mul(float4(vIn.pos.x, vIn.pos.y, vIn.pos.z, 1.f), worldViewProj);
    vOut.shadowPos	= mul(float4(vIn.pos.x, vIn.pos.y, vIn.pos.z, 1.f), lightViewProj);
	vOut.uv			= vIn.uv;
	vOut.shadowUV	= float2(temp.x, temp.y);
	vOut.normal		= mul(float4(vIn.normal, 0.f), worldView).xyz; 
	vOut.tangent	= mul(float4(vIn.tangent, 0.f), worldView).xyz;
	vOut.binormal	= mul(float4(vIn.binormal, 0.f), worldView).xyz;

	return vOut;
}

