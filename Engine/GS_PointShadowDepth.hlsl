#include "GSCommon.hlsli"

struct GSInput
{
    float4 Pos : SV_POSITION;
    uint InstanceID : SV_InstanceID;
    float Distance : DISTANCE;
    uint PointLightIndex : POINTLIGHTINDEX;
};

struct GSOutput
{
    float4 pos : SV_POSITION;
    uint renderTargetIndex : SV_RenderTargetArrayIndex;
    float Distance : DISTANCE;
};

[maxvertexcount(3)] 
void main(triangle GSInput input[3], inout TriangleStream<GSOutput> output)
{
    GSOutput element = (GSOutput)0;
    element.renderTargetIndex = (input[0].PointLightIndex * 6) + input[0].InstanceID;
        
    for (uint i = 0; i < 3; ++i)
    {
        element.pos = input[i].Pos;
        element.Distance = input[i].Distance;
            
        output.Append(element);
    }
}