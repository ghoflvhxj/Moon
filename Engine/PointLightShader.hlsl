#include "PSCommon.hlsli"

cbuffer PixelShaderConstantBuffer : register (b2)
{
	float4 g_lightPosition;		// w = Range
	float4 g_lightDirection;
	float4 g_lightColor;		// w = Power
    
    int PointLightIndex;
    
	row_major matrix g_inverseCameraViewMatrix;
	row_major matrix g_inverseProjectiveMatrix;
    row_major matrix ScreenToWorldMatrix;
};

PixelOut_LightPass main(PixelIn pIn)
{
	PixelOut_LightPass pOut = (PixelOut_LightPass)0;

	float4 depth	= g_Depth.Sample(g_Sampler, pIn.uv);
    float4 normal = g_Normal.Sample(g_Sampler, pIn.uv);
    normal.w = 0.f;
	float4 specular = g_Specular.Sample(g_Sampler, pIn.uv);

	//float3 pixelWorldPosition = PixelToWorld(pIn.uv, depth, g_inverseProjectiveMatrix, g_inverseCameraViewMatrix).xyz;
    float3 pixelWorldPosition = PixelToWorld(pIn.uv, depth, ScreenToWorldMatrix).xyz;

    float3 PointLightPos    = g_lightPosition.xyz;
    float3 deltaPosition    = PointLightPos - pixelWorldPosition.xyz;
	float3 direction		= normalize(deltaPosition);
	float3 color			= g_lightColor.xyz;
	float distance			= length(deltaPosition.xyz);
	float Range				= g_lightPosition.w;
	float intensity			= g_lightColor.w;

	//clip((distance < Range) ? 1 : -1);
	
    float a0 = 0.f;
    float a1 = 1.f;
    float a2 = 0.f;
    float attenuation = saturate((Range - distance) / (a0 + (a1 * Range) + (a2 * pow(Range, 2))));

	//-------------------------------------------------------------------------------------------------
    // Diffuse
    float3 ambient = float3(0.f, 0.f, 0.f);
    float3 normalInWorld = normalize(mul(normal, g_inverseCameraViewMatrix).xyz);
    
    float Dot = dot(normalInWorld, direction);
    float Bright = saturate(Dot); // 0 ~ 1

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

    
    float3 Direct = Bright * intensity * attenuation * (1.f - ShadowFactor);
    //float3 Direct = Bright * intensity * attenuation;
    //float3 InDirect = ambient * abs(Dot) * attenuation; // 주변광의 방향이 라이트와 일치하다는 가정하에는 동작할 듯
    pOut.lightDiffuse.xyz = color * Direct;

	//-------------------------------------------------------------------------------------------------
	// Specular
    deltaPosition = pixelWorldPosition - PointLightPos;
    direction = reflect(normalize(deltaPosition), normalInWorld.xyz);
    
    float3 CameraWorldPos = float3(g_inverseCameraViewMatrix[3][0], g_inverseCameraViewMatrix[3][1], g_inverseCameraViewMatrix[3][2]);
    float3 PixelToCamera = normalize(CameraWorldPos - pixelWorldPosition.xyz);

    float3 specularFactor = pow(saturate(dot(PixelToCamera, direction)), 10.f);
    if (Bright > 0.f)
    {
        pOut.lightSpecular = float4(specular.xyz * specularFactor * (1.f - ShadowFactor), 1.f);
    }

	return pOut;
}