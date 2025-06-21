#include "GSCommon.hlsli"

struct GSInput
{
    float4 pos      : SV_POSITION;
    float3 worldPos : POSITION0;
    float2 uv       : TEXCOORD0;
    float2 Clip     : TEXCOORD1;
    float3 normal   : NORMAL0;
    float3 tangent  : NORMAL1;
    float3 binormal : NORMAL2;
};

struct GSOutput
{
	float4 pos              : SV_POSITION;
    float3 worldPos         : POSITION0;
    float2 uv               : TEXCOORD0;
    float2 Clip             : TEXCOORD1;
    float3 normal           : NORMAL0;
    float3 tangent          : NORMAL1;
    float3 binormal         : NORMAL2;
    uint renderTargetIndex  : SV_RenderTargetArrayIndex;
};

cbuffer PointLightCBuffer : register(b2)
{
    float3 PointLightPos;
    int PointLightIndex;
    row_major matrix PointLightViewProj[6];
};

[maxvertexcount(18)] 
void main(triangle GSInput input[3], inout TriangleStream<GSOutput> output)
{
    for (uint RTIndex = 0; RTIndex < 6; ++RTIndex)
    {
        GSOutput element;
        element.renderTargetIndex = (PointLightIndex * 6) + RTIndex;
        for (uint i = 0; i < 3; ++i)
        {
            float Distance = length(PointLightPos - input[i].worldPos);
            
            element.pos         = mul(input[i].pos, PointLightViewProj[RTIndex]);
            element.worldPos    = input[i].worldPos;
            element.uv          = input[i].uv;
            element.Clip        = float2(Distance, Distance);
            element.normal      = input[i].normal;
            element.tangent     = input[i].tangent;
            element.binormal    = input[i].binormal;
            output.Append(element);
        }
        
        output.RestartStrip();
    }
}