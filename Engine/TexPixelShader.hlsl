#include "PSCommon.hlsli"

cbuffer PS_CBuffer_Texture : register(b2)
{
	bool bUseNormalTexture;
	bool bUseSpecularTexture;
    bool bAlphaMask;
};

float CalculateShadowFactor(int cascadeIndex, float4 lightspacepos)
{
    float3 projCoords = lightspacepos.xyz / lightspacepos.w;
    projCoords.x = projCoords.x * 0.5f + 0.5f;
    projCoords.y = -projCoords.y * 0.5f + 0.5f;
    if (projCoords.z > 1.f)
    {
        return 0.f;
    }
	
    float bias = 0.005f;
    float3 samplePos = float3(projCoords.x, projCoords.y, cascadeIndex);
    float shadow = 0.f;
    
    int sampeleCount = 3;
    int temp = sampeleCount / 2;
    [unroll]
    for (int x = -temp; x <= temp; ++x)
    {
        [unroll]
        for (int y = -temp; y <= temp; ++y)
        {
            float4 temp = g_ShadowDepth.SampleCmpLevelZero(g_SamplerCoparison, samplePos, projCoords.z - bias, int2(x, y));
            shadow += temp.x;
        }
    }
    shadow /= float(sampeleCount * sampeleCount);
    return shadow;
    
    //float4 temp = g_ShadowDepth.Sample(g_Sampler, samplePos);
    //return temp.x < projCoords.z - bias ? 1.f : 0.f;
}

PixelOut_GeometryPass main(PixelIn pIn)
{
	PixelOut_GeometryPass pOut;
    
	// 픽셀 쉐이더에서 SV_POSITION은 z 나누기 전의 위치 벡터이다
	// x' = x / tan(@/2)*r, z 나누기 후에는 -1<=x'<=1 의 범위를 가짐
	// y' = 1 / tan(@/2), z 나누기 후에는 -1<=y'<=1 의 범위를 가짐
	// z' = z * far/(far-near) - (near*far/(far-near)) = ((z-near)*far) / (far-near) -> 0<=z'<=far, z 나누기 후에는 0<=z'<=1 의 범위를 가짐
	// w' = 투영 행렬 곱하기 전의 z좌표
	pOut.color		= g_Diffuse.Sample(g_Sampler, pIn.uv);
	pOut.depth		= float4(pIn.pos.z / pIn.pos.w, pIn.pos.z / pIn.pos.w, pIn.pos.z / pIn.pos.w, pIn.pos.w);
	pOut.normal		= float4(pIn.normal, 1.f);  
	pOut.specular	= float4(0.f, 0.f, 0.f, 0.f);
    
    float3 normal = g_Normal.Sample(g_Sampler, pIn.uv).xyz;
    
    if (bAlphaMask)
    {
        clip(pOut.color.rgb - float3(0.13f, 0.13f, 0.13f));
    }
    
    if (true == bUseNormalTexture)
    {
        const float3x3 tanToView = float3x3(
		normalize(pIn.tangent),
		normalize(pIn.binormal),
		normalize(pIn.normal)
	);
		
        normal.x = (normal.x * 2.f) - 1.f;
        normal.y = -(normal.y * 2.f) + 1.f;
        normal.z = normal.z;
        normal = mul(normal, tanToView);

        pOut.normal = float4(normal, 1.f);
    }

	if (true == bUseSpecularTexture)
	{
		float3 specular = g_Specular.Sample(g_Sampler, pIn.uv).xyz;
		pOut.specular = float4(specular, 1.f);
	}
	

    float cascadeDistanceInCameraViewProj[3];
    for (int i = 0; i < 3; ++i)
    {
        cascadeDistanceInCameraViewProj[i] = mul(float4(0.f, 0.f, cascadeDistance[i+1], 1.f), projectionMatrix).z;
    }
    
    float4 pixelPosInLightSpace = { 0.f, 0.f, 2.f, 1.f };
    int cascadeIndex = 0;
    for (int j = 0; j < 3; ++j)
    {
        if (pIn.pos.w <= cascadeDistanceInCameraViewProj[j])
        {
            cascadeIndex = j;
            pixelPosInLightSpace = mul(float4(pIn.worldPos, 1.0f), lightViewProjMatrix[j]);
            break;
        }
    }
    
    float shadowFactor = CalculateShadowFactor(cascadeIndex, pixelPosInLightSpace);
    if (shadowFactor > 0.f)
    {
        pOut.color.xyz *= 1.f - (shadowFactor/2.f);
    }
    
    return pOut;
}

//float4 main(VertexOut vOut) : SV_TARGET
//{
//	float4 color = g_Diffuse.Sample(g_Sampler, vOut.uv);
//
//	return color;
//}