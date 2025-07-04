#include "Common.hlsli"

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

cbuffer VS_CBuffer_PerObject : register(b2)
{
	row_major matrix worldMatrix;
    row_major matrix InverseWorldMatrix;
	row_major matrix keyFrameMatrices[199];
	bool animated;
};

int getCascadeIndex(float3 pos)
{
    int cascadeIndex = 0;
    float4 posInView = mul(float4(pos, 1.f), viewMatrix);
    for (int i = 0; i < 3; ++i)
    {
        if (posInView.z <= cascadeDistance[i])
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