#include "VSCommon.hlsli"

VertexOut_Simple main(VertexIn vIn)
{
    VertexOut_Simple vOut = (VertexOut_Simple)0;

	matrix worldViewProj = mul(mul(worldMatrix, viewMatrix), projectionMatrix);
    vOut.pos = mul(float4(vIn.pos.x, vIn.pos.y, vIn.pos.z, 1.f), worldViewProj);
    vOut.color = vIn.color;
	
	return vOut;
}