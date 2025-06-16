#include "PSCommon.hlsli"

cbuffer PixelShaderConstantBuffer : register (b2)
{
	float4 g_lightPosition;		// w = Range
	float4 g_lightDirection;
	float4 g_lightColor;		// w = Power

	row_major matrix g_inverseCameraViewMatrix;
	row_major matrix g_inverseProjectiveMatrix;
};

PixelOut_LightPass main(PixelIn pIn)
{
	PixelOut_LightPass pOut;

	float4 depth	= g_Depth.Sample(g_Sampler, pIn.uv);
    float4 normal = g_Normal.Sample(g_Sampler, pIn.uv);
    normal.w = 0.f;
	float4 specular = g_Specular.Sample(g_Sampler, pIn.uv);

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

	//-------------------------------------------------------------------------------------------------
    // Diffuse
    float3 ambient = float3(1.f, 1.f, 1.f);
    float3 normalInWorld = normalize(mul(normal, g_inverseCameraViewMatrix).xyz);
    float Dot = saturate(dot(normalInWorld, direction));
    //pOut.lightDiffuse.xyz = color * (Dot * (1.f - 0.5f) + 0.5f) * intensity * attenuation;
    pOut.lightDiffuse.xyz = color * intensity;

	//-------------------------------------------------------------------------------------------------
	// Specular
    deltaPosition = pixelWorldPosition.xyz - g_lightPosition.xyz;
    direction = reflect(normalize(deltaPosition), normal.xyz);
    float3 toEye = float3(g_inverseCameraViewMatrix[3][0], g_inverseCameraViewMatrix[3][1], g_inverseCameraViewMatrix[3][2]) - pixelWorldPosition.xyz;

    float3 specularFactor = saturate(dot(direction, normalize(toEye)));
    pOut.lightSpecular = float4(specular.xyz * attenuation * specularFactor, 1.f);

    //--------------------------------------------------------------------------------------------------
    float ShadowFactor = 0.f;
    // 그림자
    for (int i = 0; i < 6; ++i)
    {
        float4 PixelPosInLightViewProj = mul(float4(pixelWorldPosition.xyz, 1.f), PointLightViewProj[i]);
        float4 PixelNDC = PixelPosInLightViewProj / PixelPosInLightViewProj.w;
        
        if (-1.f <= PixelNDC.x && PixelNDC.x <= 1.f && -1.f <= PixelNDC.y && PixelNDC.y <= 1.f && 0 <= PixelNDC.z && PixelNDC.z <= 1)
        {
            int sampleCount = 5;
            int temp = sampleCount / 2;
            int Counter = 0;

            float3 BaseDir = normalize(pixelWorldPosition.xyz - PointLightPos[0].xyz);
            [unroll]
            for (int x = -temp; x <= temp; ++x)
            {
            [unroll]
                for (int y = -temp; y <= temp; ++y)
                {
                    // ShadowDepth을 샘플링해 저장된 깊이(빛 시점에서의 깊이)와, 현재 픽셀을 깊이를 비교함
                    // 즉 샘플링한 게 더 적으면 그림자가 적용됨
                    ShadowFactor += 1.f - g_PointLightShadowDepth.SampleCmpLevelZero(g_SamplerCoparison, normalize(BaseDir + float3(x / 2048.f, y / 2048.f, 0.f)), PixelNDC.z - 0.005f).x;
                }
            }
            ShadowFactor /= sampleCount * sampleCount;
                
            //ShadowFactor = g_PointLightShadowDepth.Sample(g_Sampler, BaseDir).x < PixelNDC.z - 0.005f;

            break;
        }
    }
    
    pOut.lightDiffuse.xyz = ambient + (1.f - ShadowFactor) * pOut.lightDiffuse.xyz;
    pOut.lightDiffuse.xyz *= Dot * attenuation;
	return pOut;
}