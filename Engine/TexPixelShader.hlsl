#include "PSCommon.hlsli"

cbuffer PS_CBuffer_Texture : register(b2)
{
	bool bUseNormalTexture;
	bool bUseSpecularTexture;
    bool bAlphaMask;
};

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
	
    // 테스트 를 위한 값 초기화
    //pOut.color = float4(PixelPosInLightViewProj.z, shadowFactor, 0.f, 1.f);
    //PixelPosInLightViewProj = mul(float4(pIn.worldPos, 1.0f), lightViewProjMatrix[cascadeIndex]);
    
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