#include "PSCommon.hlsli"

PixelOut_GeometryPass main(PixelIn pIn)
{
    PixelOut_GeometryPass pOut = (PixelOut_GeometryPass)0;
    
    //pOut.color      = T_Diffuse.Sample(g_Sampler, pIn.uv);
    /***************************************
     TODO
      WIC이 모든 텍스쳐를 R8G8B8A8_UNORM 으로만 불러오는 듯
      Diffuse 텍스쳐 포맷 모디파이어를 R8G8B8A8_UNORM_SRGB로 설정하고 샘플링 하도록 변경한다면, 자동 변환이 되기 때문에 위 코드로 변경해야 함
    ***************************************/
    pOut.color      = pow(T_Diffuse.Sample(g_Sampler, pIn.uv), 2.2f);
    pOut.normal     = float4(PackNormal(pIn.normal), 1.f);

    //if (bAlphaMask)
    //{
    //    clip(pOut.color.rgb - float3(0.01f, 0.01f, 0.01f));
    //    clip(pOut.color.a - 0.001f);
    //}
    
    //if (bUseNormalTexture)
    //{
    //    float3 normal = T_Normal.Sample(g_Sampler, pIn.uv).xyz;
    //    normal = UnpackNormal(normal);
        
    //    float3x3 TBN = float3x3(pIn.tangent, pIn.binormal, pIn.normal);
    //    normal = normalize(mul(normal, TBN));
        
    //    pOut.normal = float4(PackNormal(normal.xyz), 0.f);
    //}
    
    //if (bUseEmissiveTexture)
    //{
    //    float3 Emissive = T_Emissive.Sample(g_Sampler, pIn.uv).xyz;
    //    float EmissiveScale = 1.f;
    //    pOut.Emissive = float4(Emissive * EmissiveScale, 0.f);
    //}
    
    //if (bUseSpecularTexture)
    //{
    //    float3 specular = T_Specular.Sample(g_Sampler, pIn.uv).xyz;
    //    pOut.specular = float4(float3(specular.g, specular.g, specular.g), 1.f);
    //}
    
    //if (bRimLight)
    //{
    //    pOut.RimLight = float4(1.f, 1.f, 1.f, 1.f);
    //}
	
    // 알파마스크
    float Max = max(max(pOut.color.r, pOut.color.g), pOut.color.z);
    clip(Max - 0.01f * bAlphaMask);
    clip(pOut.color.a - 0.001f * bAlphaMask);
    
    // 노말
    float3 normal = T_Normal.Sample(g_Sampler, pIn.uv).xyz;
    normal = UnpackNormal(normal);
        
    float3x3 TBN = float3x3(pIn.tangent, pIn.binormal, pIn.normal);
    normal = normalize(mul(normal, TBN));
        
    pOut.normal = float4(PackNormal(normal.xyz), 0.f);
    
    // 스펙큘러
    float3 specular = T_Specular.Sample(g_Sampler, pIn.uv).xyz;
    pOut.specular = float4(bUseSpecularTexture * float3(specular.g, specular.g, specular.g), 1.f);
    
    // 이미시브
    float3 Emissive = T_Emissive.Sample(g_Sampler, pIn.uv).xyz;
    float EmissiveScale = 1.f;
    pOut.Emissive = float4(bUseEmissiveTexture * Emissive * EmissiveScale, 0.f);
    
    // 림라이트
    pOut.RimLight = float4(bRimLight, bRimLight, bRimLight, 1.f);
    
    return pOut;
}