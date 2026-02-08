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

/*********************************
    EConstantBufferLayer와 맞아야 함
    b0 ~ b1 까지는 모든 쉐이더들이 공유해서 사용함

    b0 - 씬 전역에서 사용하며, 변경하지 않는 한 유지되는 것들
    b1 - 씬 전역에서 사용하며, 매 프레임마다 변경되는 것들
    b2 - 렌더 패스 전용
    b3 - 매터리얼 전용
    b4 - 오브젝트 전용
    b5 - 커스텀
*********************************/
#define CBUFFER_GLOBAL              b0
#define CBUFFER_TICK                b1
#define CBUFFER_RENDERPASS          b2
#define CBUFFER_RENDERPASSOBJECT    b3
#define CBUFFER_MATERIAL            b4
#define CBUFFER_OBJECT              b5
#define CBUFFER_CUSTOM              b6

cbuffer CBuffer_Global : register(CBUFFER_GLOBAL)
{
    float4 resolution;
    bool bLight;
    
    // 렌더링 옵션
    bool bDirectionalLighting = true;
    bool bPointLighting = true;
    bool bShadowing = true;
    bool bSSAO = true;
    
    // 디버깅
    bool bDebugDirectLight = false;
    bool bDebugDirectionalShadow = false;
    bool bDebugCascade = false;
    bool bDebugInDirectLight = false;
    bool bDebugSSAO = true;
    
    float NormalBiasScale = 0.1f;
    float DepthBias = 0.001f;
};

cbuffer CBuffer_Tick : register(CBUFFER_TICK)
{
    row_major matrix viewMatrix;
    row_major matrix InverseViewMatrix;
    
    row_major matrix projectionMatrix;
    row_major matrix InverseProjectionMatrix;
    row_major matrix orthographicProjectionMatrix;
    row_major matrix inverseOrthographicProjectionMatrix;
    
    row_major matrix ViewProjMatrix;
    row_major matrix InvViewProjMatrix;

	// 직교 투영용
    row_major matrix identityMatrix;
    
    float DeltaTime;
    float Time;
};

cbuffer CBuffer_Material : register(CBUFFER_MATERIAL)
{
    bool bUseNormalTexture;
    bool bUseSpecularTexture;
    bool bUseEmissiveTexture;
    bool bAlphaMask;
    bool bRimLight;
    
    float2 UVScale;
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

float3 TransformPosition(float3 InPos, float4x4 InTransformMat)
{
    return mul(float4(InPos, 1.f), InTransformMat).xyz;
}

float3 TransformNormal(float3 InNormal, float4x4 InTransformMat)
{
    return mul(float4(InNormal, 0.f), InTransformMat).xyz;
}