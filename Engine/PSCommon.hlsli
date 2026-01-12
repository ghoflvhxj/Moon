#include "Common.hlsli"

struct PixelIn
{
	float4 pos			: SV_POSITION;
    float3 worldPos		: POSITION0;
	float2 uv			: TEXCOORD0;
	float3 normal		: NORMAL0;
	float3 tangent		: NORMAL1;
	float3 binormal		: NORMAL2;
};

struct PixelIn_Simple
{
    float4 pos : SV_Position;
    float4 color : Color0;
};

struct PixelIn_SimpleTex
{
    float4 pos : SV_Position;
    float4 color : Color0;
    float2 uv : TEXCOORD0;
};

struct PixelOut_Simple
{
    float4 color : SV_TARGET0;
};

struct PixelOut_GeometryPass
{
	//float4 color	: SV_TARGET0;
	//float4 depth	: SV_TARGET1;
	//float4 normal	: SV_TARGET2;
	//float4 specular : SV_TARGET3;
    //float4 RimLight : SV_TARGET4;
    
    float4 color : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 specular : SV_TARGET2;
    float4 RimLight : SV_TARGET3;
};

struct PixelOut_CombinePass
{
	float4	color	: SV_TARGET0;
};

struct PixelOut_ShadowDepth
{
    float4 shadowDepth		: SV_TARGET0;
};

struct PixelOut_LightPass
{
	float4 lightDiffuse		: SV_TARGET0;
	float4 lightSpecular	: SV_TARGET1;
};

cbuffer PS_CBuffer_PerObject : register(b2)
{
    bool bUseNormalTexture;
    bool bUseSpecularTexture;
    bool bAlphaMask;
    bool bRimLight;
};

// 셰이더에서 사용하는 텍스쳐. 렌더 타겟인 경우는 인덱스가 ERenderTarget과 일치해야 함
Texture2D g_Diffuse				                : register(t0);
Texture2D g_Depth				                : register(t1);
Texture2D g_Normal				                : register(t2);
Texture2D g_Specular			                : register(t3); // 여기 까지가 매터리얼에 할당된 텍스쳐들

Texture2D g_LightDiffuse		                : register(t4);
Texture2D g_LightSpecular		                : register(t5);
Texture2DArray<float> g_ShadowDepth	            : register(t6);
TextureCubeArray T_PointLightDepth              : register(t7);
Texture2D T_Collision                           : register(t8);
Texture2D T_PointLightDiffuse                   : register(t9); 
Texture2D<uint2> T_Stencil                      : register(t10);
Texture2D T_Outline                             : register(t11);
Texture2D T_RimLight : register(t12);


// 셰이더에서 사용하는 샘플러
SamplerState g_Sampler : register(s0);
SamplerComparisonState g_SamplerCloser : register(s1);
SamplerComparisonState g_SamplerFarther : register(s2);

float4 PixelToView(float2 uv, float depth, matrix inverseProjectiveMatrix)
{
	// UV좌표를 (0 <= x, y <= 1) NDC좌표로 (-1 <= x, y <= 1, 단 UV좌표는 Y위 쪽이 0이다)
    float4 NDCPos = float4(0.f, 0.f, GetNear(), 0.f);
    NDCPos.x = uv.x * 2.f - 1.f;
    NDCPos.y = uv.y * -2.f + 1.f;
    NDCPos.z = depth;
    NDCPos.w = 1.f;

    float4 viewPos = mul(NDCPos, inverseProjectiveMatrix);
    return viewPos / viewPos.w;
}

float4 PixelToWorld(float2 uv, float depth, matrix InvProjMat, matrix ScreenToWorldMatrix)
{
    float3 ViewPos = PixelToView(uv, depth, InvProjMat).xyz;
    
	// NDC좌표에 역투영,뷰 행렬을 곱해 뷰 좌표를 얻음
    return mul(float4(ViewPos, 1.f), ScreenToWorldMatrix);
}

float3 PackNormal(float3 InNormal)
{
    return InNormal * 0.5 + 0.5f;
}

float3 UnpackNormal(float3 InPackedNormal)
{
    return normalize(InPackedNormal * 2.f - 1.f);
}

float3 TransformPosition(float3 InPos, float4x4 InTransformMat)
{
    return mul(float4(InPos, 1.f), InTransformMat).xyz;
}

float3 TransformNormal(float3 InNormal, float4x4 InTransformMat)
{
    return mul(float4(InNormal, 0.f), InTransformMat).xyz;
}

float2 ToUV(float2 InClipPos)
{
    /****************************************
     1. World -> View -> Proj -> NDC(-1~1, -1~1)로 만듬.
     2. 직교투영이기 때문에 나눌 z값이 1이므로 생략
     3. UV(0~1, 0~1)로 변환. 
    ****************************************/
    return float2(InClipPos.x * 0.5f + 0.5f, InClipPos.y * -0.5f + 0.5f);
}

#define SHADOW_PCF_SAMPLES 0
float PixelCascadeSahdow(int cascadeIndex, float3 InPixelWorldPos, float3 InSurfaceNormal)
{
    /**********************************************
     float Bias = lerp(0.005f, 0.05f, InSlope); -> 이렇게 하면 면과 빛의 기울기마다 bias가 다르게 되니, 일관된 bias로 적용이 안됨
     모든 면에 대해 같은 bias를 적용하되, 기울기에 따라 bias 수치를 높이고 싶은데 흠.
     기울기에 대한 Bias 스케일링은 쉐이더에서 하는게 아니라 RasterizeState에서 설정할 수 있음
     참고: https://www.gamedev.net/forums/topic/662625-slope-scale-depth-bias-shadow-map-in-hlsl/
    **********************************************/
    
    // 노말 바이어스
    float3 NormalBiasedPos = InPixelWorldPos + (InSurfaceNormal * NormalBiasScale);
    float3 NDCPos = TransformPosition(NormalBiasedPos, lightViewProjMatrix[cascadeIndex]); // 직교투영이기 때문에 ClipPos = NDCPos나 마찬가지
    
    // 단순 바이어스
    float Bias = DepthBias;
    float SurfaceDepth = DepthCloser(NDCPos.z, Bias);

    // 쉐도우 맵 뎁스 샘플링은 원래 위치를 UV로 변환
    float3 PixelPosInLightViewProj = TransformPosition(NormalBiasedPos, lightViewProjMatrix[cascadeIndex]);
    float3 ShadowDepthUV = float3(ToUV(PixelPosInLightViewProj.xy), cascadeIndex);
    float shadow = 0.f;

    if (SurfaceDepth > GetFar())
    {
#if SHADOW_PCF_SAMPLES == 0
        // 기준은 CompareValue임. s
        shadow = g_ShadowDepth.SampleCmpLevelZero(g_SamplerCloser, ShadowDepthUV, SurfaceDepth, int2(0, 0)).x;
#else
        int sampleCount = 3;
        int temp = sampleCount / 2;
        int Counter = 0;
        
        [unroll]
        for (int x = -temp; x <= temp; ++x)
        {
            [unroll]
            for (int y = -temp; y <= temp; ++y)
            {
                // ShadowDepth을 샘플링해 저장된 깊이(빛 시점에서의 깊이)와, 현재 픽셀을 깊이를 비교함
                // 즉 샘플링한 게 더 적으면 그림자가 적용됨
                
                shadow += g_ShadowDepth.SampleCmpLevelZero(g_SamplerLess, ShadowDepthUV, SurfaceDepth, int2(x, y)).x;
            }
        }
        
        shadow /= sampleCount * sampleCount;
#endif
    }
    
    return shadow;
}


float3 GetWorldPos(float2 InUV, float2 InOffset, float4x4 InInvProj, float4x4 InViewInv)
{
    float2 UV = InUV + InOffset;
    float Depth = g_Depth.Sample(g_Sampler, UV).r;
    return PixelToWorld(UV, Depth, InInvProj, InViewInv).xyz;
}