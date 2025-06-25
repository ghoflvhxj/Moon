#include "PSCommon.hlsli"

cbuffer PixelShaderConstantBuffer : register (b2)
{
	float4 g_lightPosition;		// w = Range
	float4 g_lightDirection;
	float4 g_lightColor;		// w = Power
    
    int PointLightIndex;
    
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

	float3 pixelWorldPosition = PixelToWorld(pIn.uv, depth, g_inverseProjectiveMatrix, g_inverseCameraViewMatrix).xyz;

    float3 PointLightPos    = g_lightPosition.xyz;
    float3 deltaPosition    = PointLightPos - pixelWorldPosition.xyz;
	float3 direction		= normalize(deltaPosition);
	float3 color			= g_lightColor.xyz;
	float distance			= length(deltaPosition.xyz);
	float Range				= g_lightPosition.w;
	float intensity			= g_lightColor.w;

	clip((distance < Range) ? 1 : -1);
	
    float a0 = 0.f;
    float a1 = 1.f;
    float a2 = 0.f;
    float attenuation = saturate((Range - distance) / (a0 + (a1 * Range) + (a2 * pow(Range, 2))));

	//-------------------------------------------------------------------------------------------------
    // Diffuse
    float3 ambient = float3(0.3f, 0.3f, 0.3f);
    float3 normalInWorld = normalize(mul(normal, g_inverseCameraViewMatrix).xyz);
    float Dot = saturate(dot(normalInWorld, direction));
    //pOut.lightDiffuse.xyz = color * (Dot * (1.f - 0.5f) + 0.5f) * intensity * attenuation;
    pOut.lightDiffuse.xyz = color * intensity;

	//-------------------------------------------------------------------------------------------------
	// Specular
    deltaPosition = pixelWorldPosition - PointLightPos;
    direction = reflect(normalize(deltaPosition), normal.xyz);
    float3 toEye = float3(g_inverseCameraViewMatrix[3][0], g_inverseCameraViewMatrix[3][1], g_inverseCameraViewMatrix[3][2]) - pixelWorldPosition;

    float3 specularFactor = saturate(dot(direction, normalize(toEye)));
    pOut.lightSpecular = float4(specular.xyz * attenuation * specularFactor, 1.f);

    //--------------------------------------------------------------------------------------------------
    // 그림자
    float ShadowFactor = 0.f;
    int sampleCount = 3;
    int temp = sampleCount / 2;
    int Counter = 0;

    float3 BaseDir = normalize(pixelWorldPosition - PointLightPos);
    [unroll]
    for (int x = -temp; x <= temp; ++x)
    {
    [unroll]
        for (int y = -temp; y <= temp; ++y)
        {
            // distance가 ShadowMap보다 큰가?
            ShadowFactor += T_PointLightDepth.SampleCmpLevelZero(g_SamplerGreater, float4(normalize(BaseDir + float3(x / 2048.f, y / 2048.f, 0.f)), PointLightIndex), distance - 0.005f).x;
        }
    }
    ShadowFactor /= sampleCount * sampleCount;

    pOut.lightDiffuse.xyz = (1.f - ShadowFactor) * pOut.lightDiffuse.xyz;
    pOut.lightDiffuse.xyz *= Dot * attenuation;
    
	return pOut;
}