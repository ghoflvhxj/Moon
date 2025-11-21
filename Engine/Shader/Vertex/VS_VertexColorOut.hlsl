#include "../../VSCommon.hlsli"

VertexOut_Simple main(VertexIn vIn)
{
    VertexOut_Simple vOut = (VertexOut_Simple)0;

    vOut.pos = mul(float4(vIn.pos.xyz, 1.f), WorldViewProj);
    vOut.color = vIn.color;
	
	return vOut;
}