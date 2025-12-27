#include "../../VSCommon.hlsli"

/*
    라인을 이용해 콜리전을 그림
    뎁스가 기록됨
*/

VertexOut_Simple main(VertexIn vIn, InstanceIn iIn)
{
    VertexOut_Simple VertexOut = (VertexOut_Simple)0;

    if (bInstance)
    {
        row_major matrix WorldMat = float4x4(iIn.WorldMatrixRow0, iIn.WorldMatrixRow1, iIn.WorldMatrixRow2, iIn.WorldMatrixRow3);
        matrix worldViewProj = mul(mul(WorldMat, viewMatrix), projectionMatrix);
        VertexOut.pos = mul(float4(vIn.pos.xyz, 1.f), worldViewProj);
        VertexOut.color = vIn.color;
    }
    else
    {
        matrix worldViewProj = mul(mul(worldMatrix, viewMatrix), projectionMatrix);
        VertexOut.pos = mul(float4(vIn.pos.xyz, 1.f), worldViewProj);
        VertexOut.color = vIn.color;
    }

    return VertexOut;
}