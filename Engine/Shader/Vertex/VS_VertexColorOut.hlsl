#include "../../VSCommon.hlsli"

VertexOut_Simple main(VertexIn vIn, InstanceIn iIn)
{
    VertexOut_Simple vOut = (VertexOut_Simple)0;

    if (bInstance)
    {
        row_major matrix MyWorldViewProj = float4x4(iIn.WorldMatrixRow0, iIn.WorldMatrixRow1, iIn.WorldMatrixRow2, iIn.WorldMatrixRow3);
        vOut.pos = mul(float4(vIn.pos.xyz, 1.f), MyWorldViewProj);
        vOut.color = vIn.color;
    }
    else
    {
        row_major matrix MyWorldViewProj = mul(mul(worldMatrix, viewMatrix), projectionMatrix);
        vOut.pos = mul(float4(vIn.pos.xyz, 1.f), MyWorldViewProj);
        vOut.color = vIn.color;
    }
    
	return vOut;
}