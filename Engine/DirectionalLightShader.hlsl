#include "PSCommon.hlsli"

cbuffer CBuffer : register(b2)
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

	float4 depth = g_Depth.Sample(g_Sampler, pIn.uv);
	float4 normal = g_Normal.Sample(g_Sampler, pIn.uv);
	normal.w = 0.f;
	float4 specular = g_Specular.Sample(g_Sampler, pIn.uv);

    // 그림자 계산을 위해 CascadeIndex를 구함
    int CascadeIndex = 0;
    float3 PixelPosInCameraView = PixelToView(pIn.uv, depth, g_inverseProjectiveMatrix).xyz;
    //[unroll]
    for (int i = 1; i < 4; ++i)
    {
        if (PixelPosInCameraView.z < cascadeDistance[i])
        {
            CascadeIndex = i - 1;
            break;
        }
    }
    
    // 그림자 팩터 얻기
    float4 PixelPosInWorld = mul(float4(PixelPosInCameraView, 1.f), g_inverseCameraViewMatrix);
    float4 PixelPosInLightViewProj = mul(float4(PixelPosInWorld.xyz, 1.f), lightViewProjMatrix[CascadeIndex]);
    float ShadowFactor = PixelCascadeSahdow(CascadeIndex, PixelPosInLightViewProj);

	float3 direction = normalize(g_lightDirection.xyz);
	float3 color = g_lightColor.xyz;
	float intensity = g_lightColor.w;

	//-------------------------------------------------------------------------------------------------
    float3 ambient = float3(0.5f, 0.5f, 0.5f);
	float3 normalInWorld = normalize(mul(normal, g_inverseCameraViewMatrix).xyz);
    float Dot = saturate(dot(normalInWorld, -direction)); //	float으로 해도 되는데 편할려고
    pOut.lightDiffuse.xyz = color * Dot * intensity;
    pOut.lightDiffuse.xyz = ambient + (1.f - ShadowFactor) * pOut.lightDiffuse.xyz;
    
	//-------------------------------------------------------------------------------------------------
    direction = normalize(reflect(direction, normal.xyz));
    float3 toEye = float3(g_inverseCameraViewMatrix[3][0], g_inverseCameraViewMatrix[3][1], g_inverseCameraViewMatrix[3][2]) - PixelPosInLightViewProj.xyz;

	float3 specularFactor = pow(saturate(dot(normalize(toEye), direction)), 10.f);
	pOut.lightSpecular = float4(specular.xyz * specularFactor, 1.f);

	return pOut;
}