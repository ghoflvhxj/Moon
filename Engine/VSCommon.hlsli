#include "Common.hlsli"

// Vetex.h의 InputLayoutDesc와 같아야 함.
struct VertexIn
{
	float4 pos				: POSITION0;
	float4 color			: COLOR0;
	float2 uv				: TEXCOORD0;
	float3 normal			: NORMAL0;
	float3 tangent			: NORMAL1;
	float3 binormal			: NORMAL2;
	uint4 blendIndex		: BLENDINDICES0;
	float4 blendWeight		: BLENDWEIGHT0;
};

struct InstanceIn
{
    float4 WorldMatrixRow0 : TEXCOORD1;
    float4 WorldMatrixRow1 : TEXCOORD2;
    float4 WorldMatrixRow2 : TEXCOORD3;
    float4 WorldMatrixRow3 : TEXCOORD4;
};

struct VertexOut
{
	float4 pos			: SV_POSITION;
    float3 worldPos     : POSITION0;
	float2 uv			: TEXCOORD0;
    float2 Clip         : TEXCOORD1;
    float3 normal       : NORMAL0;
    float3 tangent      : NORMAL1;
    float3 binormal     : NORMAL2;
};

struct VertexOut_Simple
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

struct VertexOut_SimpleTex
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

cbuffer VS_CBuffer_PerObject : register(b2)
{
	row_major matrix worldMatrix;
    row_major matrix WorldView;
    row_major matrix WorldViewProj;
    row_major matrix InverseWorldMatrix;
	row_major matrix keyFrameMatrices[199];
	bool animated;
    bool bOrtho;
};

int getCascadeIndex(float3 pos)
{
    int cascadeIndex = 0;
    float4 posInView = mul(float4(pos, 1.f), viewMatrix);
    for (int i = 0; i < 3; ++i)
    {
        if (posInView.z <= getComp(cascadeDistance, i))
        {
            cascadeIndex = i;
        }
        else
        {
            break;
        }
    }
	
    return cascadeIndex;
}

float4 Temp(VertexIn vIn)
{
    matrix boneTransform = identityMatrix;
    if (animated)
    {
        boneTransform = (matrix) 0;
        for (int i = 0; i < 4; ++i)
        {
            boneTransform += mul(keyFrameMatrices[vIn.blendIndex[i]], vIn.blendWeight[i]);
        }
    }
    
    float3 animatedPos = mul(float4(vIn.pos.xyz, 1.f), boneTransform).xyz;
    float4 ProjectedPos = mul(float4(animatedPos, 1.f), WorldViewProj);
    return ProjectedPos;
}