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
    float4 shadowDepth		: SV_TARGET6;
};

struct PixelOut_LightPass
{
	float4 lightDiffuse		: SV_TARGET4;
	float4 lightSpecular	: SV_TARGET5;
};

Texture2D g_Diffuse		: register(t0);
Texture2D g_Depth		: register(t1);
Texture2D g_Normal		: register(t2);
Texture2D g_Specular	: register(t3);

Texture2D g_LightDiffuse	: register(t4);
Texture2D g_LightSpecular	: register(t5);

Texture2DArray g_ShadowDepth	: register(t6);

SamplerState g_Sampler : register(s0);
SamplerComparisonState g_SamplerCoparison : register(s1);

SamplerComparisonState cmpSampler
{
   // sampler state
    Filter = COMPARISON_MIN_MAG_MIP_LINEAR;
    AddressU = MIRROR;
    AddressV = MIRROR;
 
   // sampler comparison state
    ComparisonFunc = LESS_EQUAL;
};

float4 PixelToWorld(float2 uv, float4 depth, matrix inverseProjectiveMatrix, matrix inverseCameraViewMatrix)
{
	float4 pixelProjectionPosition = float4(0.f, 0.f, 0.f, 0.f);
	pixelProjectionPosition.x = (uv.x * 2.f - 1.f) * depth.w;
	pixelProjectionPosition.y = (uv.y * -2.f + 1.f) * depth.w;
	pixelProjectionPosition.z = depth.x * depth.w * depth.w;
	pixelProjectionPosition.w = depth.w;

	float4x4 inverseProjectViewMatrix = mul(inverseProjectiveMatrix, inverseCameraViewMatrix);
	float4 pixelWorldPosition = mul(pixelProjectionPosition, inverseProjectViewMatrix);

	return pixelWorldPosition;
}