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
};

cbuffer CBuffer
{
	row_major matrix worldMatrix;
	row_major matrix viewMatrix;
	row_major matrix projectionMatrix;
};

VertexOut main( float4 pos : POSITION ) : SV_POSITION
{
	VertexOut vOut;

	matrix worldView = mul(worldMatrix, viewMatrix);
	matrix worldViewProj = mul(worldView, projectionMatrix);
	vOut.pos = mul(float4(pos.x, pos.y, pos.z, 1.f), worldViewProj);

	return vOut;
}