#include "PSCommon.hlsli"

cbuffer PixelShaderConstantBuffer : register(CBUFFER_RENDERPASS)
{
	float4 g_lightPosition;		// w = Range
	float4 g_lightColor;		// w = Power
    
    int PointLightIndex;
    
	row_major matrix g_inverseCameraViewMatrix;
	row_major matrix g_inverseProjectiveMatrix;
    row_major matrix InvProjViewMatrix;
    row_major matrix LightProjMatrix;
};

PixelOut_LightPass main(PixelIn pIn)
{
	PixelOut_LightPass pOut = (PixelOut_LightPass)0;

	float depth	= G_Depth.Sample(g_Sampler, pIn.uv).r;
    float4 normal = G_Normal.Sample(g_Sampler, pIn.uv);
    normal.xyz = UnpackNormal(normal.xyz);
    normal.w = 0.f;
	float4 specular = G_Specular.Sample(g_Sampler, pIn.uv);

    float3 PixelPosInCamera = PixelToView(pIn.uv, depth, g_inverseProjectiveMatrix).xyz;
    float3 PixelPosInWorld = TransformPosition(PixelPosInCamera, g_inverseCameraViewMatrix);
    //float3 pixelWorldPosition = PixelToWorld(pIn.uv, depth, g_inverseProjectiveMatrix, g_inverseCameraViewMatrix).xyz;

    float3 PointLightPos    = g_lightPosition.xyz;
    float3 deltaPosition    = PointLightPos - PixelPosInWorld;
	float3 direction		= normalize(deltaPosition);
	float3 color			= g_lightColor.xyz;
	float distance			= length(deltaPosition);
	float Range				= g_lightPosition.w;
	float intensity			= g_lightColor.w;

	clip((distance < Range) ? 1 : -1);
	
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
    
    float SurfaceDepth = distance;

    float3 BaseDir = normalize(PixelPosInWorld - PointLightPos);
    [unroll]
    for (int x = -temp; x <= temp; ++x)
    {
    [unroll]
        for (int y = -temp; y <= temp; ++y)
        {
            // 서페이스 뎁스(기준)가 그림자 뎁스보다 크다면, 그림자가 생겨야 함
            ShadowFactor += G_PointLightDepth.SampleCmpLevelZero(S_Greater, float4(normalize(BaseDir + float3(x / 512.f, y / 512.f, 0.f)), PointLightIndex), SurfaceDepth - 0.005f).x;
        }
    }
    ShadowFactor /= sampleCount * sampleCount;
    float NonShadow = 1.f - ShadowFactor;
    
    float3 Direct = Bright * intensity * attenuation * NonShadow;
    pOut.lightDiffuse.xyz = color * Direct;

	//-------------------------------------------------------------------------------------------------
	// Specular
    deltaPosition = PixelPosInWorld - PointLightPos;
    direction = reflect(normalize(deltaPosition), normalInWorld.xyz);
    
    float3 CameraWorldPos = float3(g_inverseCameraViewMatrix[3][0], g_inverseCameraViewMatrix[3][1], g_inverseCameraViewMatrix[3][2]);
    float3 PixelToCamera = normalize(CameraWorldPos - PixelPosInWorld);

    float3 specularFactor = pow(saturate(dot(PixelToCamera, direction)), 10.f);
    
    pOut.lightSpecular = float4(specular.xyz * specularFactor * NonShadow, 1.f);
    
    /********************************
        디버깅
    ********************************/
    //pOut.lightDiffuse.xyz = float3(SurfaceDepth, SurfaceDepth, SurfaceDepth);
    
    //float Temp = G_PointLightDepth.Sample(g_Sampler, float4(BaseDir, PointLightIndex)).x;
    //float Temp2 = SurfaceDepth - Temp;
    //pOut.lightDiffuse.xyz = float3(Temp2, Temp2, Temp2);
    
    //float Temp = G_PointLightDepth.Sample(g_Sampler, float4(BaseDir, PointLightIndex)).x;
    //float Temp2 = Temp == 1.f ? 1.f : 0.f;
    //pOut.lightDiffuse.xyz = float3(Temp2, Temp2, Temp2);
    
    //float Temp2 = G_PointLightDepth.Sample(g_Sampler, float4(BaseDir, PointLightIndex)).x / Range;
    //pOut.lightDiffuse.xyz = float3(Temp2, Temp2, Temp2);
    
    //float Temp2 = distance / Range;
    //pOut.lightDiffuse.xyz = float3(Temp2, Temp2, Temp2);
    
	return pOut;
}