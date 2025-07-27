#include "../../VSCommon.hlsli"

VertexOut_SimpleTex main(VertexIn vIn)
{
    VertexOut_SimpleTex vOut = (VertexOut_SimpleTex)0;

	matrix worldViewProj = mul(worldMatrix, bOrtho ? orthographicProjectionMatrix : projectionMatrix);
    vOut.pos = mul(float4(vIn.pos.xyz, 1.f), worldViewProj);
    vOut.color = vIn.color;
    vOut.uv = vIn.uv;
	
	return vOut;
}