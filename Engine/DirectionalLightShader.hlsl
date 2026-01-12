#include "PSCommon.hlsli"

cbuffer CBuffer : register(b2)
{
	float4 g_lightPosition;		// w = Range
	float4 g_lightDirection;
	float4 g_lightColor;		// w = Power
    float4 Ambient;
    
    row_major matrix g_inverseCameraViewMatrix;
    row_major matrix g_inverseProjectiveMatrix;
};

PixelOut_LightPass main(PixelIn pIn)
{
	PixelOut_LightPass pOut = (PixelOut_LightPass)0;

	float depth = g_Depth.Sample(g_Sampler, pIn.uv).r;
	float4 normal = g_Normal.Sample(g_Sampler, pIn.uv);
    normal.xyz = UnpackNormal(normal.xyz);
	normal.w = 0.f;
	float4 specular = g_Specular.Sample(g_Sampler, pIn.uv);

    if (all(normal.xyz == float3(0.f, 0.f, 0.f)))
    {
        return pOut;
    }
    
    float3 PixelPosInCamera = PixelToView(pIn.uv, depth, g_inverseProjectiveMatrix).xyz;
    float3 PixelPosInWorld = TransformPosition(PixelPosInCamera, g_inverseCameraViewMatrix);
    
    // 그림자 계산을 위해 CascadeIndex를 구함
    int CascadeIndex = 0;
    [unroll]
    for (int i = 1; i < 4; ++i)
    {
        if (PixelPosInCamera.z < getComp(cascadeDistance, i))
        {
            CascadeIndex = i - 1;
            break;
        }
    }
    
	float3 LightDirection = g_lightDirection.xyz;
	float3 color = g_lightColor.xyz;
	float intensity = g_lightColor.w;
    
    float3 CameraWorldPos = float3(g_inverseCameraViewMatrix[3][0], g_inverseCameraViewMatrix[3][1], g_inverseCameraViewMatrix[3][2]);
    float3 PixelToCamera = normalize(CameraWorldPos - PixelPosInWorld);
    
	//-------------------------------------------------------------------------------------------------
    // 난반사
	float3 normalInWorld = mul(normal, g_inverseCameraViewMatrix).xyz;
    float Dot = dot(normalInWorld, -LightDirection);
    float Bright = saturate(Dot);                       // 0 ~ 1
    
    float3 Direct = Bright * intensity * (1.f - ShadowFactor);
    float3 InDirect = Ambient * abs(Dot); // 주변광의 방향이 라이트와 일치하다는 가정하에는 동작할 듯
    
    float3 Direct = Bright * intensity * ShadowFactor; //(1.f - ShadowFactor);
    float3 InDirect = Ambient.xyz * abs(Dot); // 주변광의 방향이 라이트와 일치하다는 가정하에는 동작할 듯
    pOut.lightDiffuse.xyz = color * (Direct + InDirect);
    
    if(T_RimLight.Sample(g_Sampler, pIn.uv).x > 0.f)
    {
        float Rim = 1.f - saturate(dot(PixelToCamera, normalInWorld));
        Rim = pow(Rim, 20.f);
        pOut.lightDiffuse.xyz += Rim;
    }

	//-------------------------------------------------------------------------------------------------
    // 정반사
    float3 ReflectDirection = reflect(LightDirection, normalInWorld.xyz);
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
    //pOut.lightDiffuse.xyz = float3(1.f, 1.f, 1.f);
    //pOut.lightDiffuse.xyz = normal.xyz;
    //pOut.lightDiffuse.xyz = float3(depth.x, depth.x, depth.x);
    //pOut.lightDiffuse.xyz = PixelPosInCamera.xyz;
    //pOut.lightDiffuse.xyz = PixelPosInWorld.xyz;
    //pOut.lightDiffuse.xyz = PixelPosInLightViewProj.xyz;
    //pOut.lightDiffuse.xyz = normalInWorld;
    //pOut.lightDiffuse.xyz = SurfaceNormal;
    //pOut.lightDiffuse.xyz = pIn.normal; // 라이트용 메시의 노말이기 때문에 의미없음. 
    
    if (bDebugDirectionalShadow)
    {
        pOut.lightDiffuse.xyz = ShadowFactor;
    }
    
    if (bDebugCascade)
    {
        if (CascadeIndex == 0)
        {
            pOut.lightDiffuse.xyz = float3(1.f, 0.f, 0.f);
        }
        else if (CascadeIndex == 1)
        {
            pOut.lightDiffuse.xyz = float3(0.f, 1.f, 0.f);
        }
        else if (CascadeIndex == 2)
        {
            pOut.lightDiffuse.xyz = float3(0.f, 0.f, 1.f);
        }
    }

    return pOut;
}