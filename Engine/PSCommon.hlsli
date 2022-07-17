struct PixelIn
{
	float4 pos		: SV_POSITION;
	float2 uv		: TEXCOORD0;
	float3 normal	: NORMAL0;
	float3 tangent	: NORMAL1;
	float3 binormal : NORMAL2;
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

cbuffer PS_CBuffer_Texture : register(b1)
{
	bool bUseNormalTexture;
	bool bUseSpecularTexture;
};

cbuffer PixelShaderConstantBuffer : register (b2)
{
	float4 g_lightPosition;	// w = Range
	float4 g_lightColor;		// w = Power

	row_major matrix g_inverseCameraViewMatrix;
	row_major matrix g_inverseProjectiveMatrix;
};