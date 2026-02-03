#include "../../PSCommon.hlsli"

static const float2 SampleOffsets[16] =
{
    float2(0.0000, 0.0000), // 중심점
    float2(0.5373, 0.2562),
    float2(0.4374, -0.4960),
    float2(-0.4294, -0.2335),
    float2(-0.1348, 0.4730),
    float2(0.3314, 0.8401),
    float2(0.8636, -0.1117),
    float2(0.1306, -0.9026),
    float2(-0.5841, -0.7386),
    float2(-0.8572, -0.2110),
    float2(-0.7281, 0.5063),
    float2(0.0000, 0.9000),
    float2(0.6000, 0.6000),
    float2(0.2000, -0.4000), 
    float2(-0.3000, 0.7000),
    float2(0.5000, -0.1000)
};

static const float Radius = 0.2f;
static const float Bias = 0.1f;
static const float Intensity = 1.0f;
static const float Sigma = 1.0f;

cbuffer CBuffer : register(CBUFFER_RENDERPASS)
{
    //row_major matrix InvProjMatrix;
    
    //float Radius = 0.5f;
    //float Bias = 0.01f;
    //float Intensity = 1.0f;
    //float Sigma = 1.0f;
}

float AlchemySSAO(float3 InPixelPos, float3 InSamplePos, float3 InNormal)
{
    /************************
        Numerator
        cos(0):1    -> 그림자O
        cos(90):1   -> 그림자X
    
        샘플과의 거리가 멀면 그림자가 안져야 하므로 1 - x 가 1에 가까워야 함. x = 0임
        샘플과의 거리가 가까우면 그림자가 져야 하므로 1 - x 가 0에 가까워야 함. x = 1임
    ************************/
    
    float3 ToSample = InSamplePos - InPixelPos;
    
    float Numerator = max(0.f, dot(ToSample, InNormal) + (InPixelPos.z * 0.001f) - Bias);
    float Denominator = dot(ToSample, ToSample) + 0.0001f;
    
    //float Numerator = max(0.f, dot(ToSample, InNormal) + (InPixelPos.z * 0.001f) - Bias);
    //float Denominator = length(ToSample) * length(ToSample) * length(ToSample) + 0.0001f;
    
    //return Numerator / Denominator;
    
    return /*length(ToSample) > 1.f ? 0.f :*/ Numerator / Denominator;
}

float4 main(PixelIn pIn) : SV_TARGET
{
    float2 Texel = 1.f / resolution.xy;
    
    // 노말은 뷰스페이스로 저장되어 있으니 그대로 사용
    float3 Normal = G_Normal.Sample(g_Sampler, pIn.uv).xyz;
    Normal = UnpackNormal(Normal);

    float3 PixelPos = GetViewPos(pIn.uv, float2(0.f, 0.f), InverseProjectionMatrix);
    
    float Sum = 0.f;
    int SampleNum = 16;
    for (int i = 0; i < SampleNum; ++i)
    {
        //float2 UVOffset = SampleOffsets[i] * (Radius / PixelPos.z);
        float2 UVOffset = (SampleOffsets[i] * resolution.xy) * (Radius / max(1.f, PixelPos.z));
        clamp(UVOffset, Texel, 1.f);
        UVOffset /= resolution.xy;
        float3 SamplePos = GetViewPos(pIn.uv, UVOffset, InverseProjectionMatrix);

        Sum += AlchemySSAO(PixelPos, SamplePos, Normal);
    }
    
    float AO = Sum / (float) SampleNum;
    float finalAO = saturate(1.0f - (AO * Intensity));
    finalAO = pow(finalAO, Sigma);
    
    return float4(finalAO, finalAO, finalAO, 1.0f);

}

    //for (int i = 0; i < 5; ++i)
    //{
    //    for (int j = 0; j < 5; ++j)
    //    {
    //        float2 UVOffset = SampleOffsets[i] * Texel * float2(i, j);
    //        float3 SamplePos = GetViewPos(pIn.uv, UVOffset, InvProjMatrix);
          
    //        Sum += AlchemySSAO(PixelPos, SamplePos, Normal);
    //    }
    
    //}
    
    //float Sum = 0.f;
    //float a = 25.f;
    //float2 Texel = 1.f / resolution.xy;
    //for (int i = 0; i < 5; ++i)
    //{
    //    for (int j = 0; j < 5; ++j)
    //    {
    //        float2 UVOffset = Texel * float2(i, j);
    //        float3 SamplePos = GetViewPos(pIn.uv, UVOffset, InvProjMatrix);
            
    //        if (length(PixelPos - SamplePos) > 1.f)
    //        {
    //            return float4(0.f, 0.f, 0.f, 1.f);

    //        }
    //    }

    //}
    
    //return float4(1.f, 1.f, 1.f, 1.f);