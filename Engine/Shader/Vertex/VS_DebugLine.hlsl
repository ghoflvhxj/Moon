#include "../../VSCommon.hlsli"

VertexOut_Simple main(VertexIn vIn)
{
    VertexOut_Simple VertexOut = (VertexOut_Simple)0;

    matrix ViewProj = mul(viewMatrix, projectionMatrix);
    VertexOut.pos = mul(vIn.pos, ViewProj);
    VertexOut.color = vIn.color;

    return VertexOut;
}