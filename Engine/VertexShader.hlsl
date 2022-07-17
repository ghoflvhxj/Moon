#include "VSCommon.hlsli"

VertexOut_Simple main(	float3 pos : SV_POSITION,
				float4 color : COLOR)
{
	VertexOut_Simple vOut;

	matrix worldViewProj = mul(mul(worldMatrix, viewMatrix), projectionMatrix);
	vOut.pos	= mul(float4(pos.x, pos.y, pos.z, 1.f), worldViewProj);
	vOut.color	= color;
	
	return vOut;
}