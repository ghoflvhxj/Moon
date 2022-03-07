struct VertexOut
{
	float4 color : COLOR;
	float4 pos : POSITION0;
};

cbuffer CBuffer
{
	row_major matrix worldMatrix;
	row_major matrix viewMatrix;
	row_major matrix projectionMatrix;
};

VertexOut main(	float3 pos : SV_POSITION,
				float4 color : COLOR)
{
	VertexOut vOut;
	
	matrix worldViewProj = mul(mul(worldMatrix, viewMatrix), projectionMatrix);
	vOut.pos	= mul(float4(pos.x, pos.y, pos.z, 1.f), worldViewProj);
	vOut.color	= color;
	
	return vOut;
}