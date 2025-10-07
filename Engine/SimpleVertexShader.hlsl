#include "VSCommon.hlsli"

VertexOut main(VertexIn In)
{
    VertexOut Out = (VertexOut)0;

    matrix worldViewProj = mul(mul(worldMatrix, viewMatrix), projectionMatrix);

    Out.pos = mul(float4(In.pos.xyz, 1.f), worldViewProj);

	return Out;
}