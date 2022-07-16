struct VertexIn
{
	float3 pos		: POSITION0;
	float4 color	: COLOR0;
	float2 uv		: TEXCOORD0;
	float3 normal	: NORMAL0;
	float3 tangent	: NORMAL1;
	float3 binormal : NORMAL2;
};

struct VertexOut
{
	float4 pos		: SV_POSITION;
	//float4 color	: COLOR0;
	float2 uv		: TEXCOORD0;
	float3 normal	: NORMAL0;
	float3 tangent	: NORMAL1;
	float3 binormal : NORMAL2;
};

struct VertexOut_Simple
{
	float4 color : COLOR;
	float4 pos : POSITION0;
};

struct PixelOut_GeometryPass
{
	float4 color	: SV_TARGET0;
	float4 depth	: SV_TARGET1;
	float4 normal	: SV_TARGET2;
	float4 specular : SV_TARGET3;
};

struct PixelOut_CombinePass
{
	float4	color	: SV_TARGET0;
};

cbuffer VS_CBuffer_ABVD : register(b0)
{
	float resolutionWidth;
	float resolutionHeight;
};

cbuffer VS_CBuffer_PerTick : register(b1)
{
	row_major matrix viewMatrix;
	row_major matrix projectionMatrix;

	// 직교 투영용
	row_major matrix identityMatrix;
	row_major matrix orthographicProjectionMatrix;
};

cbuffer VS_CBuffer_PerObject : register(b2)
{
	row_major matrix worldMatrix;
};

//cbuffer PS_CBuffer
//{
//	bool bUseNormalTexture;
//	bool bUseSpecularTexture;
//};