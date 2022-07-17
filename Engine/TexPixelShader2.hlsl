#include "PSCommon.hlsli"

Texture2D g_Diffuse		: register(t0);
Texture2D g_Normal		: register(t1);
Texture2D g_Specular	: register(t2);

SamplerState g_Sampler;

PixelOut_GeometryPass main(PixelIn pIn)
{
	PixelOut_GeometryPass pOut;

	pOut.color		= g_Diffuse.Sample(g_Sampler, pIn.uv);

	// 픽셀 쉐이더에서 SV_POSITION의 값은
	// 0 < x < width, 0 < y < height, 0 < z < 1.f
	// 0 < w < z나누기 전의 z
	pOut.depth		= float4(pIn.pos.z / pIn.pos.w, pIn.pos.z / pIn.pos.w, pIn.pos.z / pIn.pos.w, pIn.pos.w);
	pOut.normal		= float4(pIn.normal, 0.f);
	pOut.specular	= float4(0.f, 0.f, 0.f, 0.f);

	if (true == bUseNormalTexture)
	{
		const float3x3 tanToView = float3x3(
			normalize(pIn.tangent),
			normalize(pIn.binormal),
			normalize(pIn.normal)
		);

		float3 normal = g_Normal.Sample(g_Sampler, pIn.uv).xyz;
		normal.x = (normal.x * 2.f) - 1.f;
		normal.y = -(normal.y * 2.f) + 1.f;
		normal.z = normal.z;

		//normal = pIn.normal + (normal.x * pIn.tangent) + (normal.y * pIn.binormal);
		//normal = normalize(normal);

		normal = mul(normal, tanToView);

		pOut.normal = float4(normal, 1.f);
	}

	if (true == bUseSpecularTexture)
	{
		float3 specular = g_Specular.Sample(g_Sampler, pIn.uv).xyz;
		pOut.specular = float4(specular, 1.f);
	}

	return pOut;
}