struct VertexIn
{
	float3 pos		: POSITION0;
	float4 color	: COLOR0;
	float2 uv		: TEXCOORD0;
	float3 normal	: NORMAL0;
	float3 tangent	: NORMAL1;
	float3 binormal : NORMAL2;
};

struct VertexOut
{
	float4 pos		: SV_POSITION;
	//float4 color	: COLOR0;
	float2 uv		: TEXCOORD0;
	float3 normal	: NORMAL0;
	float3 tangent	: NORMAL1;
	float3 binormal : NORMAL2;
};

cbuffer CBuffer
{
	row_major matrix worldMatrix;
	row_major matrix viewMatrix;
	row_major matrix projectionMatrix;
};

VertexOut main(VertexIn vIn)
{
	VertexOut vOut;

	matrix worldView = mul(worldMatrix, viewMatrix);
	matrix worldViewProj = mul(worldView, projectionMatrix);
	vOut.pos	= mul(float4(vIn.pos.x, vIn.pos.y, vIn.pos.z, 1.f), worldViewProj);
	//vOut.color	= vIn.color;
	vOut.uv		= vIn.uv;
	vOut.normal		= mul(vIn.normal, worldView); 
	vOut.tangent	= mul(vIn.tangent, worldView);
	vOut.binormal	= mul(vIn.binormal, worldView);

	return vOut;
}

