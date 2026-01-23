#include "PSCommon.hlsli"

/****************************************************************************************************************************
 면과 빛이 수직에 가까워 질수록, 그림자가 지면 안되는데 그림자가 짐.
 그림자 뎁스가 서페이스 뎁스보다 가깝다고 판단됬다는 것
 
 넓은 메쉬가 라이트 뷰 상에서 좁은 영역으로 변화하면서 뎁스를 기록함
 극단적으로 면이 한줄에 기록된다면 뎁스는 가까운 뎁스가 기록될 거임. 즉, 멀리 있는 부분의 뎁스가 무시된다.
 쉐도우 뎁스는 고정된 값, 서페이스 뎁스는 점점 멀어지게 됨
 
 그림자 뎁스가 서페이스 뎁스보다 가까워 진다.
 
 1. 서페이스 뎁스를 더 가깝게 만드는 방법
 2. 쉐도우 뎁스를 더 멀게 만드는 방법

 일단은 Normal Bias Offset + Depth Bias 방식을 사용
****************************************************************************************************************************/

cbuffer CBuffer : register(CBUFFER_RENDERPASS)
{
	float4 g_lightPosition;		// w = Range
	float4 g_lightDirection;
	float4 g_lightColor;		// w = Power
    float4 Ambient;
    float4 CascadeDistances;
    
    row_major matrix g_inverseCameraViewMatrix;
    row_major matrix g_inverseProjectiveMatrix;
    row_major matrix LightViewProj[4];
    row_major matrix InvProjViewMatrix;
};

PixelOut_LightPass main(PixelIn pIn)
{
	PixelOut_LightPass pOut = (PixelOut_LightPass)0;

	float depth = G_Depth.Sample(g_Sampler, pIn.uv).r;
	float4 normal = G_Normal.Sample(g_Sampler, pIn.uv);
    normal.xyz = UnpackNormal(normal.xyz);
	normal.w = 0.f;
	float4 specular = G_Specular.Sample(g_Sampler, pIn.uv);

    //if (all(normal.xyz == float3(0.f, 0.f, 0.f)))
    //{
    //    return pOut;
    //}
    bool IsNormalValid = any(normal.xyz);
    
    float3 PixelPosInCamera = PixelToView(pIn.uv, depth, g_inverseProjectiveMatrix).xyz;
    float3 PixelPosInWorld = TransformPosition(PixelPosInCamera, g_inverseCameraViewMatrix);
    
    // 그림자 계산을 위해 CascadeIndex를 구함
    int CascadeIndex = 0;
    CascadeIndex += (PixelPosInCamera.z >= CascadeDistances.y);
    CascadeIndex += (PixelPosInCamera.z >= CascadeDistances.z);
    CascadeIndex += (PixelPosInCamera.z >= CascadeDistances.w);
    
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
    
    // 면의 노말 얻기
    float3 Right = GetWorldPos(pIn.uv, float2(1.f / resolution.x, 0.f), InvProjViewMatrix);
    float3 Up = GetWorldPos(pIn.uv, float2(0.f, 1.f / resolution.y), InvProjViewMatrix);
    float3 SurfaceNormal = normalize(cross(Right - PixelPosInWorld, Up - PixelPosInWorld));
    float Temp = 1.f - saturate(dot(SurfaceNormal, -LightDirection));
    
    // 그림자 팩터 얻기
    float NonShadow = 1.f - PixelCascadeSahdow(LightViewProj[CascadeIndex], CascadeIndex, PixelPosInWorld, SurfaceNormal);
    
    float3 Direct = Bright * intensity * NonShadow;
    float3 InDirect = Ambient.xyz * abs(Dot); // 주변광의 방향이 라이트와 일치하다는 가정하에는 동작할 듯
    pOut.lightDiffuse.xyz = color * (Direct + InDirect);
    
    if(G_RimLight.Sample(g_Sampler, pIn.uv).x > 0.f)
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
        pOut.lightSpecular = float4(specular.xyz * specularFactor * NonShadow, 1.f);
    }
    
    //float3 specularFactor = saturate(dot(PixelToCamera, direction));
    //pOut.lightSpecular = float4(PixelPosInWorld.xyz, 1.f);
    
    /********************************
        디버깅용 코드
    ********************************/
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
        pOut.lightDiffuse.xyz = NonShadow;
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
    
    pOut.lightDiffuse *= IsNormalValid;
    pOut.lightSpecular *= IsNormalValid;

    return pOut;
}