#include "FullScreenQuadPass.h"

#include "StaticMeshComponent.h"
#include "MainGameSetting.h"
#include "Core/ResourceManager.h"
#include "Renderer.h"
#include "MoonEngine.h"
#include "Window.h"

MFullScreenQuadPass::MFullScreenQuadPass()
{
    Mesh::MakeRect(MeshData);
    PID = MPrimitiveComponent::MakePrimitiveID();
    
    //getRenderer()->MakeBuffer(PID, MeshData);
    getGraphicDevice()->BuildMeshBuffer(TEXT("Plane"), MeshData, 0);

    bWriteDepthStencil = false;
    bDepthEnable = false;
}

void MFullScreenQuadPass::RenderPass(const std::vector<FPrimitiveData>& PrimitiveDatList)
{
    Begin();

    FBufferContainer BufferContainer = {};
    getGraphicDevice()->GetBuffers(BufferContainer, TEXT("Plane"));

    auto& Window = getRenderer()->GetCurrentScene()->GetWindow();

    FPrimitiveData NewPrimitivData = {};
    NewPrimitivData.MeshData = &MeshData;
    NewPrimitivData.PrimitiveType = EPrimitiveType::Mesh;
    NewPrimitivData.VertexBuffer = BufferContainer.VertexBuffers[0];
    NewPrimitivData.IndexBuffer = BufferContainer.IndexBuffers[0];
    NewPrimitivData.Scale.x =  Window->GetWidth<float>();
    NewPrimitivData.Scale.y = Window->GetHeight<float>();
    NewPrimitivData.ProjectionType = EProjectionType::Orthograhpic;

    if (IsValidPrimitive(NewPrimitivData))
    {
        UpdateTickConstantBuffer(NewPrimitivData);
        UpdateObjectConstantBuffer(NewPrimitivData);
        DrawPrimitive(NewPrimitivData);
    }

    End();
}

void MCombinePass::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));
}

void MCombinePass::HandleOutputMergeStage(const FPrimitiveData& PrimitiveData)
{
    uint32 DepthStencilFalg = 0;
    DepthStencilFalg |= (uint32)Graphic::EDepthStencilMode::DepthDisable;
    DepthStencilFalg |= (uint32)Graphic::EDepthStencilMode::StencilDisable;

    g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(DepthStencilFalg), 0);
    g_pGraphicDevice->getContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void MStencilPass::HandleOutputMergeStage(const FPrimitiveData& PrimitiveData)
{
    uint32 DepthStencilFlag = 0;
    DepthStencilFlag |= (uint32)Graphic::EDepthStencilMode::DepthDisable;
    DepthStencilFlag |= (uint32)Graphic::EDepthStencilMode::StencilReadMask;

    UINT StencilRef = 1; /* PrimitiveData.PrimitiveComponent->IsStencil */
    g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(DepthStencilFlag), StencilRef);
    g_pGraphicDevice->getContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}
