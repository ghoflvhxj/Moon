#include "../../VSCommon.hlsli"

cbuffer FCapsuleData : register(b3)
{
    float Radius;
    float HalfHeight;

    int SphereVertexNum;
    int CylinderVertexNum;
};

VertexOut_Simple main(VertexIn vIn)
{
    VertexOut_Simple VertexOut = (VertexOut_Simple)0;

    matrix worldViewProj = mul(mul(worldMatrix, viewMatrix), projectionMatrix);
    VertexOut.pos = mul(float4(vIn.pos.xyz, 1.f), worldViewProj);
    VertexOut.color = vIn.color;

    return VertexOut;
}