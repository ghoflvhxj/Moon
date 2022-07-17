#include "PSCommon.hlsli"

Texture2D g_Depth		: register(t0);
Texture2D g_Normal		: register(t1);
Texture2D g_Specular	: register(t2);

SamplerState g_Sampler;

struct PixelOut
{
	float4 diffuse	: SV_TARGET0;
	float4 specular : SV_TARGET1;
};

PixelOut main(PixelIn pIn)
{
	PixelOut pOut;

	float4 depth	= g_Depth.Sample(g_Sampler, pIn.uv);
	float4 normal	= g_Normal.Sample(g_Sampler, pIn.uv);
	normal.w = 0.f;
	float4 specular = g_Specular.Sample(g_Sampler, pIn.uv);

	// uv좌표를 (0 <= x, y <= 1) 투영좌표로 (-1 <= x, y <= 1, 단 UV좌표는 Y위 쪽이 1이다)
	float4 pixelProjectionPosition = float4(0.f, 0.f, 0.f, 0.f);
	pixelProjectionPosition.x = (pIn.uv.x * 2.f - 1.f) * depth.w;
	pixelProjectionPosition.y = (pIn.uv.y * -2.f + 1.f) * depth.w;
	pixelProjectionPosition.z = depth.x * depth.w * depth.w;
	pixelProjectionPosition.w = depth.w;

	float4x4 inverseProjectViewMatrix	= mul(g_inverseProjectiveMatrix, g_inverseCameraViewMatrix);
	float4 pixelWorldPosition			= mul(pixelProjectionPosition, inverseProjectViewMatrix);

	float3 deltaPosition	= g_lightPosition.xyz - pixelWorldPosition.xyz;
	float3 direction		= normalize(deltaPosition);
	float3 color			= g_lightColor.xyz;
	float distance			= length(deltaPosition.xyz);
	float range				= g_lightPosition.w;
	float intensity			= g_lightColor.w;

	clip((distance < range) ? 1 : -1);

	float a0 = 0.f;
	float a1 = 1.f;
	float a2 = 0.f;
	float attenuation = saturate((range - distance) / (a0 + (a1 * range) + (a2 * pow(range, 2))));

	////-------------------------------------------------------------------------------------------------
	float3 normalInWorld = normalize(mul(normal, g_inverseCameraViewMatrix).xyz);
	float3 diffuseFactor = saturate(dot(normalInWorld, direction));	//	float으로 해도 되는데 편할려고
	pOut.diffuse = float4(color * diffuseFactor * attenuation * intensity, 1.f);
	//pOut.diffuse = float4(1.f - distance > 0.f ? 1.f - distance : 0.f, 0.f, 0.f, 1.f);

	//-------------------------------------------------------------------------------------------------
	deltaPosition	= pixelWorldPosition.xyz - g_lightPosition.xyz;
	direction		= reflect(normalize(deltaPosition), normal.xyz);
	float3 toEye	= float3(g_inverseCameraViewMatrix[3][0], g_inverseCameraViewMatrix[3][1], g_inverseCameraViewMatrix[3][2]) - pixelWorldPosition.xyz;

	float3 specularFactor = saturate(dot(direction, normalize(toEye)));
	pOut.specular = float4(specular.xyz * attenuation * specularFactor, 1.f);

	return pOut;
}