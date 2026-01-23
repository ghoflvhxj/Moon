#include "GSCommon.hlsli"

struct GSInput
{
    float4 Pos : SV_POSITION;
    uint InstanceID : SV_InstanceID;
    float Distance : DISTANCE;
};

struct GSOutput
{
	float4 pos      : SV_POSITION;
    uint renderTargetIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3)] 
void main(triangle GSInput input[3], inout TriangleStream<GSOutput> output)
{
    //[unroll]
    //for (uint cascadeIndex = 0; cascadeIndex < 3; ++cascadeIndex)
    //{
    //    GSOutput element = (GSOutput)0;
    //    element.renderTargetIndex = cascadeIndex;
        
    //    [unroll]
    //    for (uint i = 0; i < 3; ++i)
    //    {
    //        float4 Pos = input[i].WorldPos;
    //        element.pos = mul(Pos, lightViewProjMatrix[cascadeIndex]);
    //        output.Append(element);
    //    }
        
    //    output.RestartStrip();
    //}
    
    GSOutput element = (GSOutput) 0;
    
    element.renderTargetIndex = input[0].InstanceID;
    
    [unroll]
    for (uint i = 0; i < 3; ++i)
    {
        element.pos = input[i].Pos;
        output.Append(element);
    }
}