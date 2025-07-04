#include "VSCommon.hlsli"

VertexOut main(VertexIn vIn)
{
	VertexOut vOut;

	matrix worldView = mul(worldMatrix, viewMatrix);
	matrix worldViewProj = mul(worldView, projectionMatrix);
	
	//vOut.pos		= mul(float4(vIn.pos.x, vIn.pos.y, vIn.pos.z, 1.f), worldViewProj);
    vOut.pos		= mul(float4(vIn.pos.xyz, 1.f), worldViewProj);
    vOut.worldPos	= mul(float4(vIn.pos.xyz, 1.f), worldMatrix).xyz;
	vOut.uv			= vIn.uv;
    vOut.Clip		= vOut.pos.zw;
	vOut.normal		= mul(float4(vIn.normal, 0.f), worldView).xyz; 
	vOut.tangent	= mul(float4(vIn.tangent, 0.f), worldView).xyz;
	vOut.binormal	= mul(float4(vIn.binormal, 0.f), worldView).xyz;
    //vOut.shadowMapIndex = cascadeIndex;
	
	return vOut;
}

