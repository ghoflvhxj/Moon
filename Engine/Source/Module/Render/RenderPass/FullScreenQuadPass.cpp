#include "FullScreenQuadPass.h"
#include "StaticMeshComponent.h"
#include "MainGameSetting.h"
#include "Core/ResourceManager.h"
#include "Renderer.h"
#include "MoonEngine.h"

MFullScreenQuadPass::MFullScreenQuadPass()
{
    ViewMeshComponent = std::make_shared<StaticMeshComponent>();
    ViewMeshComponent->SetPhysics(false);
    ViewMeshComponent->SetMesh(TEXT("Base/Plane.fbx"));
    ViewMeshComponent->setTranslation(Vec3{ 0.f, 0.f, 1.f });
    ViewMeshComponent->setScale(Vec3{ g_pSetting->getResolutionWidth<float>(), g_pSetting->getResolutionHeight<float>(), 1.f });

    std::shared_ptr<MMaterial> ViewMat = std::make_shared<MMaterial>();
    ViewMeshComponent->SetMaterial(0, ViewMat);

    ViewMeshComponent->SceneComponent::Update(0.f);
    ViewMeshComponent->GetPrimitiveData(ViewPrimitiveData);
    
    getRenderer()->MakeBuffer(ViewMeshComponent);

    GetLevelChangedDelegate().Add([&]() {
        getRenderer()->MakeBuffer(ViewMeshComponent);
    });
}

void MFullScreenQuadPass::RenderPass(const std::vector<FPrimitiveData>& PrimitiveDatList)
{
    ViewPrimitiveData[0].VertexBuffer = getRenderer()->GetVertexBuffer(ViewMeshComponent->GetPrimitiveID(), 0);
    ViewPrimitiveData[0].IndexBuffer = getRenderer()->GetIndexBuffer(ViewMeshComponent->GetPrimitiveID(), 0);

    Begin();

    for (auto& PrimitiveData : ViewPrimitiveData)
    {
        if (IsValidPrimitive(PrimitiveData) == false)
        {
            continue;
        }

        UpdateTickConstantBuffer(PrimitiveData);
        UpdateObjectConstantBuffer(PrimitiveData);
        DrawPrimitive(PrimitiveData);
    }

    End();
}

void MFullScreenQuadPass::UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    MRenderPass::UpdateObjectConstantBuffer(PrimitiveData);

    std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock();

    // 패스 자체 쉐이더를 오브젝트 매터리얼 쉐이더의  Cbuffer로 수동으로 갱신해줌
    for (uint32 CbufferLayer = EnumToIndex(EConstantBufferLayer::Tick); CbufferLayer < EnumToIndex(EConstantBufferLayer::Count); ++CbufferLayer)
    {
        EConstantBufferLayer Layer = (EConstantBufferLayer)CbufferLayer;
        auto& variableInfosVS = Material->getConstantBufferVariables(ShaderType::Vertex, Layer);
        _vertexShader->UpdateConstantBuffer(Layer, variableInfosVS);
    }
}

MCombinePass::MCombinePass()
{
    bWriteDepthStencil = false;
}

void MCombinePass::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));
}

void MCombinePass::HandleOuputMergeStage(const FPrimitiveData& PrimitiveData)
{
    // DepthStencilState
    g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::EDepthWriteMode::Disable), 1);

    // OutputMergeState
    g_pGraphicDevice->getContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}
