#include "PSCommon.hlsli"

cbuffer PixelShaderConstantBuffer : register (b2)
{
	float4 g_lightPosition;		// w = Range
	float4 g_lightDirection;
	float4 g_lightColor;		// w = Power

	row_major matrix g_CameraViewMatrix;
	row_major matrix g_inverseCameraViewMatrix;
	row_major matrix g_inverseProjectiveMatrix;
};

PixelOut_LightPass main(PixelIn pIn)
{
	PixelOut_LightPass pOut;

	float4 depth	= g_Depth.Sample(g_Sampler, pIn.uv);
	float4 normal	= g_Normal.Sample(g_Sampler, pIn.uv);
	normal.w = 0.f;
	float4 specular = g_Specular.Sample(g_Sampler, pIn.uv);

	// uv좌표를 (0 <= x, y <= 1) 투영좌표로 (-1 <= x, y <= 1, 단 UV좌표는 Y위 쪽이 1이다)
	//float4 pixelProjectionPosition = float4(0.f, 0.f, 0.f, 0.f);
	//pixelProjectionPosition.x = (pIn.uv.x * 2.f - 1.f) * depth.w;
	//pixelProjectionPosition.y = (pIn.uv.y * -2.f + 1.f) * depth.w;
	//pixelProjectionPosition.z = depth.x * depth.w * depth.w;
	//pixelProjectionPosition.w = depth.w;

	//float4x4 inverseProjectViewMatrix	= mul(g_inverseProjectiveMatrix, g_inverseCameraViewMatrix);
	//float4 pixelWorldPosition			= mul(pixelProjectionPosition, inverseProjectViewMatrix);
	float4 pixelWorldPosition = PixelToWorld(pIn.uv, depth, g_inverseProjectiveMatrix, g_inverseCameraViewMatrix);

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
	// Diffuse
	float3 normalInWorld = normalize(mul(normal, g_inverseCameraViewMatrix).xyz);
	float diffuseFactor = saturate(dot(normalInWorld, direction));
	pOut.lightDiffuse = float4(color * diffuseFactor * attenuation * intensity, 1.f);

	//-------------------------------------------------------------------------------------------------
	// Specular
	//deltaPosition	= pixelWorldPosition.xyz - g_lightPosition.xyz;
	//direction		= reflect(normalize(deltaPosition), normal.xyz);
	//float3 toEye	= float3(g_inverseCameraViewMatrix[3][0], g_inverseCameraViewMatrix[3][1], g_inverseCameraViewMatrix[3][2]) - pixelWorldPosition.xyz;

	//float3 specularFactor = saturate(dot(direction, normalize(toEye)));
	//pOut.lightSpecular = float4(specular.xyz * attenuation * specularFactor, 1.f);

	return pOut;
}