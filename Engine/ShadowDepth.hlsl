#include "VSCommon.hlsli"

VertexOut main(VertexIn vIn)
{
    VertexOut vOut;
    
    matrix worldViewProj = mul(mul(worldMatrix, viewMatrixForShadow), projectionMatrix);
    vOut.pos = mul(float4(vIn.pos, 1.f), worldViewProj);
    
    return vOut;
}