SamplerState g_Sampler;

struct PixelIn
{
	float4 pos		: SV_POSITION;
	//float4 color	: COLOR0;
	float2 uv		: TEXCOORD0;
	float3 normal	: NORMAL0;
	float3 tangent	: NORMAL1;
	float3 binormal : NORMAL2;
};

struct PixelOut
{
	float4 color	: SV_TARGET0;
	float4 depth	: SV_TARGET1;
	float4 light	: SV_TARGET2;
};

cbuffer CBuffer
{
	bool bUseNormalTexture;
	bool bUseSpecularTexture;
};

PixelOut main(PixelIn pIn)
{
	PixelOut pOut;
	pOut.color = float4(1.f, 0.f, 0.f, 1.f);
	pOut.depth = float4(pIn.pos.z / pIn.pos.w, pIn.pos.z / pIn.pos.w, pIn.pos.z / pIn.pos.w, pIn.pos.w);;
	pOut.light = float4(1.f, 1.f, 1.f, 1.f);

	return pOut;

}