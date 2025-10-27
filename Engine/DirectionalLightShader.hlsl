#include "PSCommon.hlsli"

cbuffer CBuffer : register(b2)
{
	float4 g_lightPosition;		// w = Range
	float4 g_lightDirection;
	float4 g_lightColor;		// w = Power
	
    row_major matrix g_inverseCameraViewMatrix;
    row_major matrix g_inverseProjectiveMatrix;
};

PixelOut_LightPass main(PixelIn pIn)
{
	PixelOut_LightPass pOut = (PixelOut_LightPass)0;

	float4 depth = g_Depth.Sample(g_Sampler, pIn.uv);
	float4 normal = g_Normal.Sample(g_Sampler, pIn.uv);
	normal.w = 0.f;
	float4 specular = g_Specular.Sample(g_Sampler, pIn.uv);

    if (all(normal.xyz == float3(0.f, 0.f, 0.f)))
    {
        return pOut;
    }
    
    // 그림자 계산을 위해 CascadeIndex를 구함
    int CascadeIndex = 0;
    float3 PixelPosInCameraView = PixelToView(pIn.uv, depth, g_inverseProjectiveMatrix).xyz;
    [unroll]
    for (int i = 1; i < 4; ++i)
    {
        if (PixelPosInCameraView.z < getComp(cascadeDistance, i))
        {
            CascadeIndex = i - 1;
            break;
        }
    }
    
    // 그림자 팩터 얻기
    float4 PixelPosInWorld = mul(float4(PixelPosInCameraView, 1.f), g_inverseCameraViewMatrix);
    float4 PixelPosInLightViewProj = mul(float4(PixelPosInWorld.xyz, 1.f), lightViewProjMatrix[CascadeIndex]);
    float ShadowFactor = PixelCascadeSahdow(CascadeIndex, PixelPosInLightViewProj);

	float3 LightDirection = normalize(g_lightDirection.xyz);
	float3 color = g_lightColor.xyz;
	float intensity = g_lightColor.w;
    
    float3 CameraWorldPos = float3(g_inverseCameraViewMatrix[3][0], g_inverseCameraViewMatrix[3][1], g_inverseCameraViewMatrix[3][2]);
    float3 PixelToCamera = normalize(CameraWorldPos - PixelPosInWorld.xyz);
    
	//-------------------------------------------------------------------------------------------------
    // 난반사
    float3 ambient = float3(0.1f, 0.1f, 0.1f);
	float3 normalInWorld = normalize(mul(normal, g_inverseCameraViewMatrix).xyz);
    
    float Dot = dot(normalInWorld, -LightDirection);
    float Bright = saturate(Dot);                       // 0 ~ 1
    
    float3 Direct = Bright * intensity * (1.f - ShadowFactor);
    float3 InDirect = ambient * abs(Dot); // 주변광의 방향이 라이트와 일치하다는 가정하에는 동작할 듯
    pOut.lightDiffuse.xyz = color * (Direct + InDirect);
    
    if(T_RimLight.Sample(g_Sampler, pIn.uv).x > 0.f)
    {
        float Rim = 1.f - saturate(dot(PixelToCamera, normalInWorld));
        Rim = pow(Rim, 2.f);
        pOut.lightDiffuse.xyz += Rim;
    }

	//-------------------------------------------------------------------------------------------------
    // 정반사
    float3 ReflectDirection = normalize(reflect(LightDirection, normalInWorld.xyz));
    float3 specularFactor = pow(saturate(dot(PixelToCamera, ReflectDirection)), 10.f);
    if (Bright > 0.f)
    {
        pOut.lightSpecular = float4(specular.xyz * specularFactor * (1.f - ShadowFactor), 1.f);
    }
    
    //float3 specularFactor = saturate(dot(PixelToCamera, direction));
    //pOut.lightSpecular = float4(PixelPosInWorld.xyz, 1.f);
    
    /*
        디버깅용 코드
    */
    //pOut.lightDiffuse.xyz = PixelPosInWorld.xyz;
    //pOut.lightDiffuse.xyz = PixelPosInCameraView.xyz;
    //pOut.lightDiffuse.xyz = PixelPosInLightViewProj.z;
    //if (CascadeIndex == 0)
    //{
    //    pOut.lightDiffuse.x = 1.f;
    //}
    //else if(CascadeIndex == 1)
    //{
    //    pOut.lightDiffuse.y = 1.f;
    //}
    //else if (CascadeIndex == 2)
    //{
    //    pOut.lightDiffuse.z = 1.f;
    //}

    return pOut;
}