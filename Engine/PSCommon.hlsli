#include "Common.hlsli"

struct PixelIn
{
	float4 pos			: SV_POSITION;
    float3 worldPos		: POSITION0;
	float2 uv			: TEXCOORD0;
    float2 Clip			: TEXCOORD1;
	float3 normal		: NORMAL0;
	float3 tangent		: NORMAL1;
	float3 binormal		: NORMAL2;
};

struct PixelIn_Simple
{
    float4 pos : SV_Position;
    float4 color : Color0;
};

struct PixelOut_GeometryPass
{
	float4 color	: SV_TARGET0;
	float4 depth	: SV_TARGET1;
	float4 normal	: SV_TARGET2;
	float4 specular : SV_TARGET3;
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

// 셰이더에서 사용하는 텍스쳐. 렌더 타겟인 경우는 인덱스가 ERenderTarget과 일치해야 함
Texture2D g_Diffuse				                : register(t0);
Texture2D g_Depth				                : register(t1);
Texture2D g_Normal				                : register(t2);
Texture2D g_Specular			                : register(t3);
Texture2D g_LightDiffuse		                : register(t4);
Texture2D g_LightSpecular		                : register(t5);
Texture2DArray g_ShadowDepth	                : register(t6);
TextureCube<float> g_PointLightShadowDepth      : register(t7);
Texture2D T_Collision                           : register(t8);
Texture2D T_PointLightDiffuse                   : register(t9);   

// 셰이더에서 사용하는 샘플러
SamplerState g_Sampler : register(s0);
SamplerComparisonState g_SamplerCoparison : register(s1);

// 픽셀 좌표를 월드 좌표로 변환하는 함수
float4 PixelToWorld(float2 uv, float4 depth, matrix inverseProjectiveMatrix, matrix inverseCameraViewMatrix)
{
	// 역투영, uv좌표를 (0 <= x, y <= 1) 투영좌표로 (-1 <= x, y <= 1, 단 UV좌표는 Y위 쪽이 1이다)
	float4 pixelProjectionPosition = float4(0.f, 0.f, 0.f, 0.f);
	pixelProjectionPosition.x = (uv.x * 2.f - 1.f) * depth.w;
	pixelProjectionPosition.y = (uv.y * -2.f + 1.f) * depth.w;
    pixelProjectionPosition.z = depth.x * depth.w;
	pixelProjectionPosition.w = depth.w;

	// 투영좌표에 역투영&뷰 행렬을 곱해 월드 좌표를 얻음
	float4x4 inverseProjectViewMatrix = mul(inverseProjectiveMatrix, inverseCameraViewMatrix);
	float4 pixelWorldPosition = mul(pixelProjectionPosition, inverseProjectViewMatrix);

	return pixelWorldPosition;
}

float4 PixelToView(float2 uv, float4 depth, matrix inverseProjectiveMatrix)
{
    // 역투영, uv좌표를 (0 <= x, y <= 1) 투영좌표로 (-1 <= x, y <= 1, 단 UV좌표는 Y위 쪽이 1이다)
    float4 pixelProjectionPosition = float4(0.f, 0.f, 0.f, 0.f);
    pixelProjectionPosition.x = (uv.x * 2.f - 1.f) * depth.w;
    pixelProjectionPosition.y = (uv.y * -2.f + 1.f) * depth.w;
    pixelProjectionPosition.z = depth.x * depth.w;
    pixelProjectionPosition.w = depth.w;

	// 투영좌표에 역투영&뷰 행렬을 곱해 월드 좌표를 얻음
    float4 pixelWorldPosition = mul(pixelProjectionPosition, inverseProjectiveMatrix);
    return pixelWorldPosition;
}

// 그림자가 없으면 1, 있으면 1보다 작을거임
float PixelCascadeSahdow(int cascadeIndex, float4 PixelPosInLightViewProj)
{
    // 픽셀의 투영 좌표계 위치를 NDC(-1~1, -1~1)로 만들고(직교투영은 생략), UV(0~1, 0~1)로 변환. 
    //float3 projCoords = lightspacepos.xyz / lightspacepos.w;
    float shadow = 0.f;
    float3 ShadowDepthUV = float3(PixelPosInLightViewProj.x * 0.5f + 0.5f, PixelPosInLightViewProj.y * -0.5f + 0.5f, cascadeIndex);
    saturate(ShadowDepthUV);

    float bias = 0.005f;
    float Depth = PixelPosInLightViewProj.z - bias;

    if (Depth < 1.f)
    {
        int sampleCount = 5;
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
                
                shadow += 1.f - g_ShadowDepth.SampleCmpLevelZero(g_SamplerCoparison, ShadowDepthUV, Depth, int2(x, y)).x;
            }
        }
        
        shadow /= sampleCount * sampleCount;
    }
    
    return shadow;
}