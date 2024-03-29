#include "PSCommon.hlsli"

cbuffer CBuffer : register(b2)
{
	float4 g_lightPosition;		// w = Range
	float4 g_lightDirection;
	float4 g_lightColor;		// w = Power
	
	row_major matrix g_CameraViewMatrix;
	row_major float4x4 g_inverseCameraViewMatrix;
	row_major float4x4 g_inverseProjectiveMatrix;
};

PixelOut_LightPass main(PixelIn pIn)
{
	PixelOut_LightPass pOut;

	float4 depth = g_Depth.Sample(g_Sampler, pIn.uv);
	float4 normal = g_Normal.Sample(g_Sampler, pIn.uv);
	normal.w = 0.f;
	float4 specular = g_Specular.Sample(g_Sampler, pIn.uv);

	// uv좌표를 (0 <= x, y <= 1) 투영좌표로 (-1 <= x, y <= 1, 단 UV좌표는 Y위 쪽이 1이다)
	float4 pixelProjectionPosition = float4(0.f, 0.f, 0.f, 0.f);
	pixelProjectionPosition.x = (pIn.uv.x * 2.f - 1.f) * depth.w;
	pixelProjectionPosition.y = (pIn.uv.y * -2.f + 1.f) * depth.w;
	pixelProjectionPosition.z = depth.x * depth.w * depth.w;
	pixelProjectionPosition.w = depth.w;

	float4x4 inverseProjectViewMatrix = mul(g_inverseProjectiveMatrix, g_inverseCameraViewMatrix);
	float4 pixelWorldPosition = mul(pixelProjectionPosition, inverseProjectViewMatrix);

	float3 direction = normalize(g_lightDirection.xyz);
	float3 color = g_lightColor.xyz;
	float intensity = g_lightColor.w;

	//-------------------------------------------------------------------------------------------------
	float3 normalInWorld = normalize(mul(normal, g_inverseCameraViewMatrix).xyz);
	float diffuseFactor = saturate(dot(normalInWorld, -direction));	//	float으로 해도 되는데 편할려고
	pOut.lightDiffuse = float4(color * diffuseFactor * intensity, 1.f);

	//-------------------------------------------------------------------------------------------------
	direction = normalize(reflect(direction, normal.xyz));
	float3 toEye = float3(g_inverseCameraViewMatrix[3][0], g_inverseCameraViewMatrix[3][1], g_inverseCameraViewMatrix[3][2]) - pixelWorldPosition.xyz;

	float3 specularFactor = pow(saturate(dot(normalize(toEye), direction)), 10.f);
	pOut.lightSpecular = float4(specular.xyz * specularFactor, 1.f);

	return pOut;
}