#include "VSCommon.hlsli"

VertexOut main(VertexIn vIn)
{
    VertexOut vOut = (VertexOut)0;

    matrix worldView = mul(worldMatrix, identityMatrix);
    matrix worldViewProj = mul(worldView, orthographicProjectionMatrix);
    vOut.pos = mul(float4(vIn.pos.x, vIn.pos.y, vIn.pos.z, 1.f), worldViewProj);
    
    //vOut.pos = Temp(vIn);
	vOut.uv			= vIn.uv;
    vOut.Clip		= float2(0.f, 0.f);
    vOut.normal     = mul(float4(vIn.normal, 0.f), WorldView).xyz;
    vOut.tangent    = mul(float4(vIn.tangent, 0.f), WorldView).xyz;
    vOut.binormal   = mul(float4(vIn.binormal, 0.f), WorldView).xyz;

	return vOut;
}

