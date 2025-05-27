struct VertexOut
{
	float4 pos		: SV_POSITION;
	float4 color	: COLOR;
};

float4 main(float3 color : Color) : SV_TARGET0
{
	return float4(color, 1.f);
}