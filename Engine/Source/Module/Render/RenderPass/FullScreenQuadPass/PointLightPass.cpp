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

void PointLightPass::Begin()
{
    MFullScreenQuadPass::Begin();

    // TODO. Directional 패스에서 그린 Specular 를 지우지 않도록 임시 수정. 개선해야 함
    auto& ViewBindData = RenderTargetViewData[0];
    auto& RenderTarget = getRenderer()->GetRenderTarget(RenderTargetViewData[0].Index);

    getGraphicDevice()->ClearRenderTarget(RenderTarget, Color);
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

void PointLightPass::UpdateRenderPassConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    std::shared_ptr<MPointLightComponent> LightComp = PrimitiveData.GetPrimitiveComponent<MPointLightComponent>();
    std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock();

    Vec3 trans = LightComp->getWorldTranslation();
    Vec4 transAndRange = { trans.x, trans.y, trans.z, LightComp->getRange() };
    Vec4 color = { 1.f, 1.f, 1.f, 1.f };
    color.x = LightComp->getColor().x;
    color.y = LightComp->getColor().y;
    color.z = LightComp->getColor().z;
    color.w = LightComp->getIntensity();

    auto& Camera = getRenderer()->GetWorld()->getMainCamera();

    if (std::shared_ptr<MShader>& PixelShader = Material->getPixelShader())
    {
        PixelShader->SetValue(TEXT("g_lightPosition"), transAndRange);
        PixelShader->SetValue(TEXT("g_lightColor"), color);
        PixelShader->SetValue(TEXT("PointLightIndex"), Indexer++);

        PixelShader->SetValue(TEXT("g_inverseCameraViewMatrix"), Camera->getInvesrViewMatrix());
        PixelShader->SetValue(TEXT("g_inverseProjectiveMatrix"), Camera->getInversePerspectiveProjectionMatrix());

        Mat4 ProjMat = {};

        float Near = GraphicDevice::bReverseDepth ? 1000.f : 0.1f;
        float Far = GraphicDevice::bReverseDepth ? 0.1f : 1000.f;

        XMStoreFloat4x4(&ProjMat, XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), 1.f, Near, Far));
        PixelShader->SetValue(TEXT("LightProjMatrix"), ProjMat);

        Mat4 Mat = {};
        XMMATRIX XMMat = XMLoadFloat4x4(&Camera->getInversePerspectiveProjectionMatrix()) * XMLoadFloat4x4(&Camera->getInvesrViewMatrix());
        XMStoreFloat4x4(&Mat, XMMat);
        PixelShader->SetValue(TEXT("InvProjViewMatrix"), Mat);
    }

    MRenderPass::UpdateRenderPassConstantBuffer(PrimitiveData);
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

        LightPrimitiveData->PrimitiveComponent.lock()->setScale(NewPrimitiveData.Scale);
        LightPrimitiveData->PrimitiveComponent.lock()->Update(0.f);

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
