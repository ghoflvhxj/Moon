#include "PointLightPass.h"
#include "MoonEngine.h"
#include "Renderer.h"
#include "GraphicDevice.h"

#include "Camera.h"
#include "Material.h"
#include "PointLightComponent.h"

PointLightPass::PointLightPass()
    : MFullScreenQuadPass()
{
}

void PointLightPass::End()
{
    MRenderPass::End();
    Indexer = 0;
}

bool PointLightPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    //return false;
    return PrimitiveData.PrimitiveType == EPrimitiveType::PointLight && MRenderPass::IsValidPrimitive(PrimitiveData);
}

void PointLightPass::UpdateRenderPassObjectConstantBuffer(std::shared_ptr<MShader> InShader, const FPrimitiveData& PrimitiveData)
{
    MFullScreenQuadPass::UpdateRenderPassObjectConstantBuffer(InShader, PrimitiveData);

    std::shared_ptr<MPointLightComponent> LightComp = PrimitiveData.GetPrimitiveComponent<MPointLightComponent>();

    if (InShader->IsPixelShader())
    {
        Vec3 trans = LightComp->getWorldTranslation();
        Vec4 transAndRange = { trans.x, trans.y, trans.z, LightComp->getRange() };
        Vec4 color = { 1.f, 1.f, 1.f, 1.f };
        color.x = LightComp->getColor().x;
        color.y = LightComp->getColor().y;
        color.z = LightComp->getColor().z;
        color.w = LightComp->getIntensity();

        InShader->SetValue(TEXT("g_lightPosition"), transAndRange);
        InShader->SetValue(TEXT("g_lightColor"), color);
        InShader->SetValue(TEXT("PointLightIndex"), Indexer++);

        Mat4 ProjMat = {};

        float Near = GraphicDevice::bReverseDepth ? 1000.f : 0.1f;
        float Far = GraphicDevice::bReverseDepth ? 0.1f : 1000.f;

        XMStoreFloat4x4(&ProjMat, XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), 1.f, Near, Far));
        InShader->SetValue(TEXT("LightProjMatrix"), ProjMat);

        InShader->SetValue(TEXT("bLightShadowing"), LightComp->IsShadowing());
    }
}

std::vector<FPrimitiveData> PointLightPass::MakePrimitiveDatas()
{
    std::vector<FPrimitiveData> PrimitiveDatas;

    auto& LightPrimitiveDatas = getRenderer()->GetPrimitiveDatas(EPrimitiveType::PointLight);
    for (uint32 i=0; i<GetSize(LightPrimitiveDatas); ++i)
    {
        const auto& LightPrimitiveData = LightPrimitiveDatas[i];

        FPrimitiveData NewPrimitiveData = CreatePrimitiveData(EPrimitiveType::PointLight);
        NewPrimitiveData.PrimitiveComponent = LightPrimitiveData->PrimitiveComponent;
        NewPrimitiveData.Material = LightPrimitiveData->Material;
        NewPrimitiveData.bUseCustomTransform = true;

        PrimitiveDatas.push_back(NewPrimitiveData);
    }

    return PrimitiveDatas;
}

void PointLightPass::HandleOutputMergeStage(const FPrimitiveData& primitiveData)
{
    uint32 DepthStencilFlag = 0;
    DepthStencilFlag |= (uint32)Graphic::EDepthStencilMode::DepthDisable;
    DepthStencilFlag |= (uint32)Graphic::EDepthStencilMode::StencilDisable;

    g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(DepthStencilFlag), 0);
    g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Light), nullptr, 0xffffffff);
}
