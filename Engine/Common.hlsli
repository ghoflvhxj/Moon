#ifndef __FXAA_FILTER_FX__
#define __FXAA_FILTER_FX__
#define FXAA_PC 1
#define FXAA_HLSL_5 1
#endif

cbuffer CBuffer_ABVD : register(b0)
{
    float4 resolution;
    bool bLight;
};

cbuffer CBuffer_PerTick : register(b1)
{
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;

	// 직교 투영용
    row_major matrix identityMatrix;
    row_major matrix orthographicProjectionMatrix;
    row_major matrix inverseOrthographicProjectionMatrix;
    
    // 디렉셔널 라이트
    float4 lightPos[3];
    row_major matrix lightViewProjMatrix[3];
    float4 cascadeDistance;
};