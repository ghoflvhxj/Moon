#ifndef __FXAA_FILTER_FX__
#define __FXAA_FILTER_FX__
#define FXAA_PC 1
#define FXAA_HLSL_5 1
#endif

#define REVERSE_DEPTH 1

/*********************************
 b# - 상수버퍼 0 ~ 13
 t# - 쉐이더 리소스 뷰(텍스쳐, 구조화된 버퍼 등) 0 ~ 127
 u# - 언오더드 액세스 뷰 0 ~ 7
*********************************/

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
    
    float NormalBiasScale = 0.1f;
    float DepthBias = 0.001f;
    
    bool bDebugDirectionalLight = false;
    bool bDebugDirectionalShadow = false;
    bool bDebugCascade = false;
};

inline float GetNear()
{
#if REVERSE_DEPTH == 1
    return 1.f;
#else 
    return 0.f;
#endif
}

inline float GetFar()
{
#if REVERSE_DEPTH == 1
    return 0.f;
#else 
    return 1.f;
#endif
}

inline float ToNear()
{
    /************************************************
                                Near
     Normal     0.5 -> 0.4      0
     Reverse    0.4 -> 0.5      1
    ************************************************/
#if REVERSE_DEPTH == 1
    return 1.f;
#else 
    return -1.f;
#endif
}

// NDC 공간에서 z를 가깝게 만듬
inline float DepthCloser(float InBase, float InBias)
{
    /**************************************
     Near < NDC.z << Far
    **************************************/
    return InBase + ToNear() * InBias;
}

float getComp(float4 v, int i)
{
    if (i == 0)
        return v.x;
    if (i == 1)
        return v.y;
    if (i == 2)
        return v.z;
    return v.w;
}