#include "GSCommon.hlsli"

struct GSInput
{
    float4 pos      : SV_POSITION;
    float3 worldPos : POSITION0;
    float2 uv       : TEXCOORD0;
    float3 normal   : NORMAL0;
    float3 tangent  : NORMAL1;
    float3 binormal : NORMAL2;
};

struct GSOutput
{
	float4 pos          : SV_POSITION;
    float3 worldPos     : POSITION0;
    float2 uv           : TEXCOORD0;
    float3 normal       : NORMAL0;
    float3 tangent      : NORMAL1;
    float3 binormal     : NORMAL2;
    uint renderTargetIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(9)] 
void main(triangle GSInput input[3], inout TriangleStream<GSOutput> output)
{
    for (uint cascadeIndex = 0; cascadeIndex < 2; ++cascadeIndex)
    {
        GSOutput element;
        element.renderTargetIndex = cascadeIndex;
        for (uint i = 0; i < 3; ++i)
        {
            element.pos         = mul(input[i].pos, lightViewProjMatrix[cascadeIndex]);
            element.worldPos    = input[i].worldPos;
            element.uv          = input[i].uv;
            element.normal      = input[i].normal;
            element.tangent     = input[i].tangent;
            element.binormal    = input[i].binormal;
            //element.shadowMapIndex = input[i].shadowMapIndex;
            output.Append(element);
        }
        
        output.RestartStrip();
    }
}