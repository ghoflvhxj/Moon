#include "VSCommon.hlsli"

VertexOut main(VertexIn vIn, InstanceIn iIn)
{
	VertexOut vOut;

    vOut.pos		= LocalToProj(float4(vIn.pos.xyz, 1.f));
    vOut.worldPos	= mul(float4(vIn.pos.xyz, 1.f), worldMatrix).xyz;
    vOut.uv         = vIn.uv * float2(ScaleU, ScaleV);
    
    vOut.normal = mul(float4(vIn.normal, 0.f), WorldView).xyz;
    vOut.tangent = mul(float4(vIn.tangent, 0.f), WorldView).xyz;
    vOut.binormal = mul(float4(vIn.binormal, 0.f), WorldView).xyz;
	
	return vOut;
}

