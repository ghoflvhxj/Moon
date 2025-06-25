struct VIN
{
    float4 pos : POSITION0;
};

struct VertexOut
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float2 uv : TEXCOORD0;
    float2 Clip : TEXCOORD1;
    float3 normal : NORMAL0;
    float3 tangent : NORMAL1;
    float3 binormal : NORMAL2;
};


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
};

cbuffer VS_CBuffer_PerObject : register(b2)
{
    row_major matrix worldMatrix;
    row_major matrix InverseWorldMatrix;
    row_major matrix keyFrameMatrices[199];
    bool animated;
};


VertexOut main(VIN In)
{
    VertexOut Out = (VertexOut)0;

    matrix worldViewProj = mul(mul(worldMatrix, viewMatrix), projectionMatrix);

    Out.pos = mul(float4(In.pos.xyz, 1.f), worldViewProj);

	return Out;
}