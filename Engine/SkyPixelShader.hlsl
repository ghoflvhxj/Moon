Texture2D g_Diffuse : register(t0);
Texture2D g_Depth	: register(t1);

SamplerState g_Sampler;

struct PixelIn
{
	float4 pos		: SV_POSITION;
	float2 uv		: TEXCOORD0;
	float3 normal	: NORMAL0;
	float3 tangent	: NORMAL1;
	float3 binormal : NORMAL2;
};

struct PixelOut
{
	float4 color	: SV_TARGET0;
	float4 light	: SV_TARGET1;
};

cbuffer CBuffer
{
	bool bUseNormalTexture;
	bool bUseSpecularTexture; 
};

PixelOut main(PixelIn pIn)
{
	PixelOut pOut;

	// ��ī�� �� �޽��� uv��ǥ�� �̿��ϴ°� �ƴ϶�, Depth ���� Ÿ�� �޽��� uv��ǥ�� �̿��ؾ� ��
	// �׷��� Depth ���� Ÿ���� ũ��� �ػ󵵶� ����
	float width = 0.f;
	float height = 0.f;
	g_Depth.GetDimensions(width, height);

	float4 depth = g_Depth.Sample(g_Sampler, float2(pIn.pos.x / width, pIn.pos.y / height));

	clip((0.0 == depth.r) ? 1 : -1);

	pOut.color = g_Diffuse.Sample(g_Sampler, pIn.uv);
	pOut.light = float4(1.f, 1.f, 1.f, 0.f);

	return pOut;
}