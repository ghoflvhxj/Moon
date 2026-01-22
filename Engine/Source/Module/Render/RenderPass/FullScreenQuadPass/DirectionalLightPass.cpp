#include "DirectionalLightPass.h"
#include "MoonEngine.h"
#include "Renderer.h"

#include "Camera.h"
#include "Material.h"
#include "DirectionalLightComponent.h"

DirectionalLightPass::DirectionalLightPass()
    : MFullScreenQuadPass()
{
}

void DirectionalLightPass::UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    auto& PrimitiveComponent = PrimitiveData.PrimitiveComponent.lock()->CastToShared<MDirectionalLightComponent>();
    std::shared_ptr<MShader>& PixelShader = GetPixelShader(PrimitiveData);

    Vec3 trans = PrimitiveComponent->getWorldTranslation();
    Vec4 transAndRange = { trans.x, trans.y, trans.z, 10.f };
    Vec4 ColorAndIntensity = { PrimitiveComponent->getColor().x, PrimitiveComponent->getColor().y, PrimitiveComponent->getColor().z, PrimitiveComponent->getIntensity() };
    const Vec3& Direction = PrimitiveComponent->GetDirection();

    PixelShader->SetValue(TEXT("g_lightPosition"), transAndRange);
    PixelShader->SetValue(TEXT("g_lightDirection"), Direction);
    PixelShader->SetValue(TEXT("g_lightColor"), ColorAndIntensity);
    PixelShader->SetValue(TEXT("g_inverseCameraViewMatrix"), getRenderer()->GetWorld()->getMainCamera()->getInvesrViewMatrix());
    PixelShader->SetValue(TEXT("g_inverseProjectiveMatrix"), getRenderer()->GetWorld()->getMainCamera()->getInversePerspectiveProjectionMatrix());
    PixelShader->SetValue(TEXT("Ambient"), PrimitiveComponent->GetAmbient());

    MRenderPass::UpdateObjectConstantBuffer(PrimitiveData);
}

std::vector<FPrimitiveData> DirectionalLightPass::MakePrimitiveDatas()
{
    std::vector<FPrimitiveData> PrimitiveDatas;

    auto& LightPrimitiveDatas = getRenderer()->GetPrimitiveDatas(EPrimitiveType::DirectionalLight);
    for (const auto& LightPrimitiveData : LightPrimitiveDatas)
    {
        FPrimitiveData NewPrimitiveData = CreatePrimitiveData(EPrimitiveType::DirectionalLight);
        NewPrimitiveData.PrimitiveComponent = LightPrimitiveData->PrimitiveComponent;
        NewPrimitiveData.Material = LightPrimitiveData->Material;

        LightPrimitiveData->PrimitiveComponent.lock()->setScale(NewPrimitiveData.Scale);
        LightPrimitiveData->PrimitiveComponent.lock()->Update(0.f);

        PrimitiveDatas.push_back(NewPrimitiveData);
    }

    return PrimitiveDatas;
}

bool DirectionalLightPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    return PrimitiveData.PrimitiveType == EPrimitiveType::DirectionalLight && MRenderPass::IsValidPrimitive(PrimitiveData);
}

void DirectionalLightPass::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->ShadowDepthRS.Get());
}

void DirectionalLightPass::HandleOutputMergeStage(const FPrimitiveData& primitiveData)
{
    uint32 DepthStencilFlag = 0;
    DepthStencilFlag |= (uint32)Graphic::EDepthStencilMode::DepthDisable;
    DepthStencilFlag |= (uint32)Graphic::EDepthStencilMode::StencilDisable;

    g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(DepthStencilFlag), 0);
    g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Light), nullptr, 0xffffffff);
}
