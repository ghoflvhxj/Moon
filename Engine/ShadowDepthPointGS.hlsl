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

[maxvertexcount(18)] 
void main(triangle GSInput input[3], inout TriangleStream<GSOutput> output)
{
    for (uint RTIndex = 0; RTIndex < 6; ++RTIndex)
    {
        GSOutput element;
        element.renderTargetIndex = RTIndex;
        for (uint i = 0; i < 3; ++i)
        {
            element.pos         = mul(input[i].pos, PointLightViewProj[RTIndex]);
            element.worldPos    = input[i].worldPos;
            element.uv          = input[i].uv;
            element.Clip        = element.pos.zw;
            element.normal      = input[i].normal;
            element.tangent     = input[i].tangent;
            element.binormal    = input[i].binormal;
            output.Append(element);
        }
        
        output.RestartStrip();
    }
}