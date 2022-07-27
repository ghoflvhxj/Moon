#include "PSCommon.hlsli"

struct PixelOut
{
	float4 color	: SV_TARGET0;
	float4 light	: SV_TARGET1;
};

cbuffer CBuffer
{
	bool bUseNormalTexture;
	bool bUseSpecularTexture; 
};

PixelOut main(PixelIn pIn)
{
	PixelOut pOut;

	// 스카이 돔 메쉬의 uv좌표를 이용하는게 아니라, Depth 렌더 타겟 메쉬의 uv좌표를 이용해야 함
	// 그런데 Depth 렌더 타겟의 크기는 해상도랑 같음
	float4 depth = g_Depth.Sample(g_Sampler, float2(pIn.pos.x / resolutionWidth, pIn.pos.y / resolutionHeight));

	clip((0.0 == depth.r) ? 1 : -1);

	pOut.color = g_Diffuse.Sample(g_Sampler, pIn.uv);
	pOut.light = float4(1.f, 1.f, 1.f, 0.f);

	return pOut;
}