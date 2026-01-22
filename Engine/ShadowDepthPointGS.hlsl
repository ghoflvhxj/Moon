#include "GSCommon.hlsli"

struct GSInput
{
    float4 WorldPos : SV_POSITION;
};

struct GSOutput
{
	float4 pos              : SV_POSITION;
    uint renderTargetIndex  : SV_RenderTargetArrayIndex;
    float4 Distance : POSITION0;
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
        GSOutput element = (GSOutput)0;
        element.renderTargetIndex = (PointLightIndex * 6) + RTIndex;
        
        for (uint i = 0; i < 3; ++i)
        {
            float Distance = length(PointLightPos - input[i].WorldPos.xyz);
            
            element.pos = mul(input[i].WorldPos, PointLightViewProj[RTIndex]);
            element.Distance = float4(Distance, Distance, Distance, 1.f);
            output.Append(element);
        }
        
        output.RestartStrip();
    }
}