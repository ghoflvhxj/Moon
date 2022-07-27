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

struct PixelOut_LightPass
{
	float4 lightDiffuse		: SV_TARGET4;
	float4 lightSpecular	: SV_TARGET5;
};

cbuffer PS_CBuffer_Constancy : register(b0)
{
	float resolutionWidth;
	float resolutionHeight;
};

Texture2D g_Diffuse		: register(t0);
Texture2D g_Depth		: register(t1);
Texture2D g_Normal		: register(t2);
Texture2D g_Specular	: register(t3);

Texture2D g_LightDiffuse	: register(t4);
Texture2D g_LightSpecular	: register(t5);



SamplerState g_Sampler;