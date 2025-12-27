#include "../../VSCommon.hlsli"

VertexOut_Simple main(VertexIn vIn, InstanceIn iIn)
{
    VertexOut_Simple vOut = (VertexOut_Simple)0;

    row_major matrix WorldMat = float4x4(iIn.WorldMatrixRow0, iIn.WorldMatrixRow1, iIn.WorldMatrixRow2, iIn.WorldMatrixRow3);
    
    vOut.pos = mul(float4(vIn.pos.xyz, 1.f), WorldMat);
    vOut.color = vIn.color;
	
	return vOut;
}