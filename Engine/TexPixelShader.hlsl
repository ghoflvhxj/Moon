#include "PSCommon.hlsli"

cbuffer PS_CBuffer_Texture : register(b2)
{
	bool bUseNormalTexture;
	bool bUseSpecularTexture;
    bool bAlphaMask;
};

float CalculateShadowFactor(int cascadeIndex, float4 PixelPosInLightViewProj)
{
    // 픽셀의 투영 좌표계 위치를 NDC(-1~1, -1~1)로 만들고(직교투영은 생략), UV(0~1, 0~1)로 변환. 
    //float3 projCoords = lightspacepos.xyz / lightspacepos.w;
    float shadow = 0.f;
    float3 ShadowDepthUV = float3(PixelPosInLightViewProj.x * 0.5f + 0.5f, PixelPosInLightViewProj.y * -0.5f + 0.5f, cascadeIndex);
    saturate(ShadowDepthUV);

    float bias = 0.005f;
    float Depth = PixelPosInLightViewProj.z - bias;

    if (Depth < 1.f)
    {
        int sampleCount = 3;
        int temp = sampleCount / 2;
        int Counter = 0;
        
        [unroll]
        for (int x = -temp; x <= temp; ++x)
        {
            [unroll]
            for (int y = -temp; y <= temp; ++y)
            {
                // ShadowDepth을 샘플링해 저장된 깊이(빛 시점에서의 깊이)와, 현재 픽셀을 깊이를 비교함
                // 즉 샘플링한 게 더 적으면 그림자가 적용됨
                
                shadow += 1.f - g_ShadowDepth.SampleCmpLevelZero(g_SamplerCoparison, ShadowDepthUV, Depth, int2(x, y)).x;
            }
        }
        
        shadow /= sampleCount * sampleCount;
        //shadow = ShadowDepthUV.x;
        //shadow = g_ShadowDepth.Sample(g_Sampler, ShadowDepthUV).r;
        //shadow = g_ShadowDepth.Sample(g_Sampler, ShadowDepthUV).r < Depth ? 1.f : 0.f;
        //shadow = g_ShadowDepth.SampleCmpLevelZero(g_SamplerCoparison, ShadowDepthUV, Depth);
    }
    

    return shadow;
}

PixelOut_GeometryPass main(PixelIn pIn)
{
	PixelOut_GeometryPass pOut;
    
    float NDCDepth = pIn.Clip.x / pIn.Clip.y;
    
	pOut.color		= g_Diffuse.Sample(g_Sampler, pIn.uv);
    pOut.depth      = float4(NDCDepth, NDCDepth, NDCDepth, pIn.Clip.y);
	pOut.normal		= float4(pIn.normal, 1.f);  
	pOut.specular	= float4(0.f, 0.f, 0.f, 0.f);

    if (bAlphaMask)
    {
        clip(pOut.color.rgb - float3(0.13f, 0.13f, 0.13f));
        pOut.color.rgb = smoothstep(0.f, 1.f, pOut.color.rgb);
    }
    
    if (true == bUseNormalTexture)
    {
        float3 normal = g_Normal.Sample(g_Sampler, pIn.uv).xyz;
        normal = normalize(normal * 2.f - 1.f);
        
        float3x3 TBN = float3x3(pIn.tangent, pIn.binormal, pIn.normal);
        normal = normalize(mul(normal, TBN));
        
        pOut.normal = float4(normal, 1.f);
    }

	if (true == bUseSpecularTexture)
	{
		float3 specular = g_Specular.Sample(g_Sampler, pIn.uv).xyz;
		pOut.specular = float4(specular, 1.f);
	}
	
    // 이 픽셀의 cascade 단계를 찾음
    int cascadeIndex = 0;
    for (int j = 1; j < 4; ++j)
    {
        // 깊이 값을 비교해서 크다면, 단계를 찾은 것.
        if (pIn.Clip.y <= cascadeDistance[j])
        {
            cascadeIndex = j - 1;
            break;
        }
    }
    
    // 디렉셔널 라이트 그림자
    // 픽셀의 월드 위치를 빛의 시점의 직교투영 좌표계로 변환
    //float4 PixelPosInLightViewProj = mul(float4(pIn.worldPos, 1.0f), lightViewProjMatrix[cascadeIndex]);
    float4 PixelPosInLightViewProj = mul(float4(pIn.worldPos, 1.0f), lightViewProjMatrix[cascadeIndex]);
    float shadowFactor = CalculateShadowFactor(cascadeIndex, PixelPosInLightViewProj);
    //if (shadowFactor > 0.f)
    {   
        // 그림자가 없으면 1이고, 있으면 1보다 작을거임
        pOut.color.xyz *= 1.f - CalculateShadowFactor(cascadeIndex, PixelPosInLightViewProj);
    }
    
    // 포인트 라이트 그림자
    for (int i = 0; i < 6; ++i)
    {
        PixelPosInLightViewProj = mul(float4(pIn.worldPos, 1.f), PointLightViewProj[i]);
        float4 PixelNDC = PixelPosInLightViewProj / PixelPosInLightViewProj.w;
        
        if (-1.f <= PixelNDC.x && PixelNDC.x <= 1.f && -1.f <= PixelNDC.y && PixelNDC.y <= 1.f && 0 <= PixelNDC.z && PixelNDC.z <= 1)
        {
            float ShadowDepth = g_PointLightShadowDepth.Sample(g_Sampler, normalize(pIn.worldPos - PointLightPos[0].xyz)).x;
            //pOut.color.xyz = ShadowDepth;
            if (ShadowDepth < PixelNDC.z - 0.01f)
            {
                pOut.color.xyz *= 0.f;
            }
            
            break;
        }
    }
        
    
    //pOut.color = float4(PixelPosInLightViewProj.z, shadowFactor, 0.f, 1.f);
    
    /*
    깊이 값 비교 테스트
    */
    //float3 ShadowDepthUV = float3(PixelPosInLightViewProj.x * 0.5f + 0.5f, PixelPosInLightViewProj.y * -0.5f + 0.5f, cascadeIndex);
    //pOut.color = float4(PixelPosInLightViewProj.z, g_ShadowDepth.Sample(g_Sampler, ShadowDepthUV).r, 0.f, 1.f);
    
    /*
    쉐오두 값 테스트
    */
    //float3 ShadowDepthUV = float3(PixelPosInLightViewProj.x * 0.5f + 0.5f, PixelPosInLightViewProj.y * -0.5f + 0.5f, cascadeIndex);
    //pOut.color = float4(ShadowDepthUV, g_ShadowDepth.Sample(g_Sampler, ShadowDepthUV).r);
    //pOut.color = float4(ShadowDepthUV, cascadeIndex);
    
    /*
    쉐도우 UV 테스트
    */
    //float3 ShadowDepthUV = float3(PixelPosInLightViewProj.x * 0.5f + 0.5f, PixelPosInLightViewProj.y * -0.5f + 0.5f, cascadeIndex);
    //pOut.color = float4(ShadowDepthUV, 1.f);
    
    /* 
    쉐도우 맵 지형에 그리기 테스트
    지형이 그림자 맵을 그대로 그려야 함
    근데 카메라가 움직이면 그림자 맵의 위치가 바뀌는데 흠?
    */
    //float3 ShadowDepthUV = float3(PixelPosInLightViewProj.x * 0.5f + 0.5f, PixelPosInLightViewProj.y * -0.5f + 0.5f, cascadeIndex);
    //pOut.color = float4(g_ShadowDepth.Sample(g_Sampler, ShadowDepthUV).x, PixelPosInLightViewProj.z, 0.f, 1.f);
    
    /*
    픽셀의 월드 좌표 테스트
    */
    //pOut.color = float4(pIn.worldPos, 1.f); // 성공
    
    /*
    빛 시점의 투영 좌표계에서 픽셀의 위치 테스트. 
    직교투영이니 바로 NDC임.
    x는 -1 ~ 1 범위이니 +1 해줘서 봐야 함, 왼쪽이 검정 오른쪽이 빨강
    y는 -1 ~ 1 범위이니 +1 해줘서 봐야 함, 위쪽이 빨강 아래쪽이 검정
    카메라가 움직일 때마다 빛의 위치가 다르게 되니, x는 좌우, y는 상하로 움직이면서 테스트 해야 함
    */
    //pOut.color = float4(PixelPosInLightViewProj.z, 0.f, 0.f, 1.f); //성공, -1 ~ 1 범위가 맞음
    
    /*
    빛의 위치 테스트
    */
    //pOut.color = float4(lightPos[0].xyz, 1.f); 카메라 위치가 변하니 frustum 위치도 변경됨. 따라서 라이트 포지션도 변경되는게 맞음
    
    /*
    Cascade 테스트
    */
    //if (cascadeIndex == 0)
    //{
    //    pOut.color = float4(1.f, 0.f, 0.f, 1.f); // 첫번째 캐스케이드
    //}
    //else if(cascadeIndex == 1)
    //{
    //    pOut.color = float4(0.f, 1.f, 0.f, 1.f); // 두번째 캐스케이드
    //}
    //else if(cascadeIndex == 2)
    //{
    //    pOut.color = float4(0.f, 0.f, 1.f, 1.f); // 세번째 캐스케이드
    //}
    //else if(cascadeIndex == 3)
    //{
    //    pOut.color = float4(1.f, 1.f, 0.f, 1.f); // 네번째 캐스케이드
    //}
    
    return pOut;
}

//float4 main(VertexOut vOut) : SV_TARGET
//{
//	float4 color = g_Diffuse.Sample(g_Sampler, vOut.uv);
//
//	return color;
//}