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
    float4 color : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 specular : SV_TARGET2;
    float4 Emissive : SV_TARGET3;
    float4 RimLight : SV_TARGET4;
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
    float4 DirectDiffuse : SV_TARGET0;
    float4 DirectSpecular : SV_TARGET1;
    float4 InDirectDiffuse : SV_TARGET2;
};

// 텍스쳐. ETextureType과 일치해야 함
Texture2D T_Diffuse				                : register(t0);
Texture2D T_Dummy                               : register(t1);
Texture2D T_Normal				                : register(t2);
Texture2D T_Specular			                : register(t3);
Texture2D T_Emissive                            : register(t4);

// 렌더 타겟. ERenderTarget 과 일치해야 함
Texture2D G_Diffuse                             : register(t10);
Texture2D G_Normal                              : register(t11);
Texture2D G_Specular                            : register(t12);
Texture2D G_Emissive                            : register(t13);

Texture2D G_Depth				                : register(t20);
Texture2DArray<float> G_ShadowDepth	            : register(t21);
TextureCubeArray G_PointLightDepth              : register(t22);

Texture2D G_LightDirectDiffuse                  : register(t23);
Texture2D G_LightDirectSpecular                 : register(t24);
Texture2D G_IndirectDiffuse                     : register(t25);

Texture2D G_Collision                           : register(t30); 
Texture2D<uint2> T_Stencil                      : register(t31);
Texture2D G_Outline                             : register(t32);
Texture2D G_RimLight                            : register(t33);

Texture2D G_EmissiveDownSampled                 : register(t40);
Texture2D G_EmissiveBlurRow                     : register(t41);
Texture2D G_EmissiveBlurCol                     : register(t42);
Texture2D G_EmissiveUpSampled                   : register(t43);

Texture2D G_SSAO : register(t50);
Texture2D G_SSAODownSample : register(t51);
Texture2D G_SSAOBlurRow : register(t52);
Texture2D G_SSAOBlurCol : register(t53);
Texture2D G_SSAOUpSample : register(t54);

// 셰이더에서 사용하는 샘플러
SamplerState g_Sampler : register(s0);
SamplerState S_Linear : register(s1);
SamplerComparisonState g_SamplerCloser : register(s2);
SamplerComparisonState g_SamplerFarther : register(s3);

SamplerComparisonState S_Greater : register(s4);


#define RENDERPASS_STURCTURED_BUFFER t100

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

float4 PixelToWorld(float2 uv, float depth, matrix InvProjMat, matrix InvViewMat)
{
    float4 NDCPos = float4(0.f, 0.f, GetNear(), 0.f);
    NDCPos.x = uv.x * 2.f - 1.f;
    NDCPos.y = uv.y * -2.f + 1.f;
    NDCPos.z = depth;
    NDCPos.w = 1.f;
    
    matrix InvViewProjMat = mul(InvProjMat, InvViewMat);
    float4 WorldPos = mul(NDCPos, InvViewProjMat);
    WorldPos = WorldPos / WorldPos.w;
	// NDC좌표에 역투영,뷰 행렬을 곱해 뷰 좌표를 얻음
    return WorldPos;
}

float4 PixelToWorld(float2 uv, float depth, matrix InInvProjViewMat)
{
    float4 NDCPos = float4(0.f, 0.f, GetNear(), 0.f);
    NDCPos.x = uv.x * 2.f - 1.f;
    NDCPos.y = uv.y * -2.f + 1.f;
    NDCPos.z = depth;
    NDCPos.w = 1.f;
    
    float4 WorldPos = mul(NDCPos, InInvProjViewMat);
    WorldPos = WorldPos / WorldPos.w;
	// NDC좌표에 역투영,뷰 행렬을 곱해 뷰 좌표를 얻음
    return WorldPos;
}

float3 PackNormal(float3 InNormal)
{
    return InNormal * 0.5 + 0.5f;
}

float3 UnpackNormal(float3 InPackedNormal)
{
    return normalize(InPackedNormal * 2.f - 1.f);
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

#define SHADOW_PCF_SAMPLES 1
float PixelCascadeSahdow(row_major matrix InLightViewProj, int cascadeIndex, float3 InPixelWorldPos, float3 InSurfaceNormal)
{
    /**********************************************
     float Bias = lerp(0.005f, 0.05f, InSlope); -> 이렇게 하면 면과 빛의 기울기마다 bias가 다르게 되니, 일관된 bias로 적용이 안됨
     모든 면에 대해 같은 bias를 적용하되, 기울기에 따라 bias 수치를 높이고 싶은데 흠.
     기울기에 대한 Bias 스케일링은 쉐이더에서 하는게 아니라 RasterizeState에서 설정할 수 있음
     참고: https://www.gamedev.net/forums/topic/662625-slope-scale-depth-bias-shadow-map-in-hlsl/
    **********************************************/
    
    // 노말 바이어스
    float3 NormalBiasedPos = InPixelWorldPos + (InSurfaceNormal * NormalBiasScale);
    float3 NDCPos = TransformPosition(NormalBiasedPos, InLightViewProj); // 직교투영이기 때문에 ClipPos = NDCPos나 마찬가지
    
    // 단순 바이어스
    float Bias = DepthBias;
    float SurfaceDepth = DepthCloser(NDCPos.z, Bias);

    // 쉐도우 맵 뎁스 샘플링은 원래 위치를 UV로 변환
    float3 PixelPosInLightViewProj = TransformPosition(NormalBiasedPos, InLightViewProj);
    float3 ShadowDepthUV = float3(ToUV(PixelPosInLightViewProj.xy), cascadeIndex);
    float shadow = 0.f;

    if (SurfaceDepth > GetFar())
    {
#if SHADOW_PCF_SAMPLES == 0
        // 기준은 CompareValue임. s
        shadow = G_ShadowDepth.SampleCmpLevelZero(g_SamplerCloser, ShadowDepthUV, SurfaceDepth, int2(0, 0)).x;
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
                /***********************************************************
                 서페이스 뎁스(기준)가 쉐도우 뎁스보다 멀다면 그림자.
                ***********************************************************/
                shadow += G_ShadowDepth.SampleCmpLevelZero(g_SamplerFarther, ShadowDepthUV, SurfaceDepth, int2(x, y)).x;
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
    float Depth = G_Depth.Sample(g_Sampler, UV).r;
    return PixelToWorld(UV, Depth, InInvProj, InViewInv).xyz;
}

float3 GetWorldPos(float2 InUV, float2 InOffset, float4x4 InInvProjView)
{
    float2 UV = InUV + InOffset;
    float Depth = G_Depth.Sample(g_Sampler, UV).r;
    return PixelToWorld(UV, Depth, InInvProjView).xyz;
}

float3 GetViewPos(float2 InUV, float2 InOffset, float4x4 InInvProj)
{
    float2 UV = InUV + InOffset;
    float Depth = G_Depth.Sample(g_Sampler, UV).r;
    return PixelToView(UV, Depth, InInvProj).xyz;
}

float3 BoxBlur(Texture2D InTexture, float2 InUV, int2 InBoxSize)
{
    float Width = 0, Height = 0;
    InTexture.GetDimensions(Width, Height);
    
    float TexelWidth = 1.f / Width;
    float TexelHeight = 1.f / Height;
    
    int HalfWidth = InBoxSize.x / 2;
    int HalfHeight = InBoxSize.y / 2;
    
    float3 Sum = float3(0.f, 0.f, 0.f);
    float SampleNum = 0.f;
    for (int i = -HalfWidth; i <= HalfWidth; ++i)
    {
        for (int j = -HalfHeight; j <= HalfHeight; ++j)
        {
            float3 Color = InTexture.Sample(S_Linear, InUV + float2(i * TexelWidth, j * TexelHeight)).xyz;
            float lum = dot(Color, float3(0.2126, 0.7152, 0.0722));
            if (lum > 0.1f)
            {
                Sum += Color;
                SampleNum += 1.f;
            }
        }
    }
    
    if (SampleNum <= 0.0f)
        return float3(0.0f, 0.0f, 0.0f);
    
    //return Sum / SampleNum;
    return Sum / float(InBoxSize.x * InBoxSize.y);
}

//StructuredBuffer<float> Kernal : register(t99);

float2 UVToSnappedTexel(float2 InUV, float2 InTextureSize)
{
    return (floor(InUV * InTextureSize) + 0.5f) / InTextureSize;
}

float3 GaussianBlur(Texture2D InTexture, float2 InUV, StructuredBuffer<float> InWeights, bool bInRow)
{
    //float Weight[11] = { 0.009, 0.027, 0.065, 0.121, 0.176, 0.204, 0.176, 0.121, 0.065, 0.027, 0.009 };
    //int KernalWidth = 11;
    
    uint KernalWidth = 0, Stride = 0;
    InWeights.GetDimensions(KernalWidth, Stride);
    
    float2 TexSize = float2(0.f, 0.f);
    InTexture.GetDimensions(TexSize.x, TexSize.y);
    
    float TexelWidth = 1.f / TexSize.x;
    float TexelHeight = 1.f / TexSize.y;
    
    float3 BlurColor = float3(0.f, 0.f, 0.f);

    /*
    총 10 * 10 픽셀이라면
    
    한번에 평균을 구할 경우
     한 픽셀의 평균은 5x5 픽셀 합/25 = 덧셈:24 나눗셈:1 총:25
     이미지에 적용하면 25 * 100 = 2500번 계산
     총 2500번 계산

    가로 세로를 분리할 경우
     가로의 한 픽셀의 평균은, 5픽셀 합/5 = 덧셈:4 나눗셈:1 총:5
     이미지에 적용하면 5 * 100 = 500번 계산

     세로의 한 픽셀의 평균은, 5픽셀 합/5 = 덧셈:4 나눗셈:1 총:5
     이미지에 적용하면 5 * 100 = 500번 계산
     총 1000번 계산
    */

    // 가로
    if (bInRow)
    {
        int HalfWidth = (int)(KernalWidth / 2);
        for (int i = -HalfWidth; i <= HalfWidth; ++i)
        {
            float2 SnappedUV = UVToSnappedTexel(InUV + float2(TexelWidth * i, 0.f), TexSize);
            float3 Color = InTexture.Sample(S_Linear, SnappedUV).xyz;
            
            //float lum = dot(Color, float3(0.2126, 0.7152, 0.0722));

            //float knee = 1.f * 0.5; // 0.3~0.6 추천
            //float soft = saturate((lum - 1.f + knee) / knee);
            //float contrib = max(lum - 1.f, 0) + soft * soft;

            //float3 bright = Color * contrib;
            
            //BlurColor += bright * Weight[i + HalfWidth];
            
            BlurColor += Color * InWeights[i + HalfWidth];
        }
    }
    else
    {
        int HalfHeight = (int)(KernalWidth / 2);
        for (int i = -HalfHeight; i <= HalfHeight; ++i)
        {
            float2 SnappedUV = UVToSnappedTexel(InUV + float2(0.f, TexelHeight * i), TexSize);
            float3 Color = InTexture.Sample(S_Linear, SnappedUV).xyz;
            
            //float lum = dot(Color, float3(0.2126, 0.7152, 0.0722));

            //float knee = 1.f * 0.5; // 0.3~0.6 추천
            //float soft = saturate((lum - 1.f + knee) / knee);
            //float contrib = max(lum - 1.f, 0) + soft * soft;

            //float3 bright = Color * contrib;
            
            //BlurColor += bright * Weight[i + HalfHeight];
            
            BlurColor += Color * InWeights[i + HalfHeight];
        }
    }

    return BlurColor;
}