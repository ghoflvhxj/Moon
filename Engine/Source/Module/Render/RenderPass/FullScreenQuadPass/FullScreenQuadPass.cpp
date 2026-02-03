#include "FullScreenQuadPass.h"

#include "Module/Render/Scene.h"
#include "StaticMeshComponent.h"
#include "MainGameSetting.h"
#include "Core/ResourceManager.h"
#include "Renderer.h"
#include "MoonEngine.h"
#include "Window.h"

MFullScreenQuadPass::MFullScreenQuadPass()
{
    Mesh::MakeRect(MeshData);
    getGraphicDevice()->BuildMeshBuffer(TEXT("Plane"), MeshData, 0, true);

    bWriteDepthStencil = false;
    bDepthEnable = false;
}

void MFullScreenQuadPass::RenderPass(std::vector<FPrimitiveData>& PrimitiveDatList)
{
    std::vector<FPrimitiveData>& PrimitiveDatas = MakePrimitiveDatas();
    
    //for (const FPrimitiveData& PrimitiveData : PrimitiveDatas)
    //{
    //    if (IsValidPrimitive(PrimitiveData) == false)
    //    {
    //        continue;
    //    }

    //    UpdateRenderPassConstantBuffer(PrimitiveData);
    //    UpdateMaterialConstantBuffer(PrimitiveData.Material.lock(), PrimitiveData);
    //    UpdateObjectConstantBuffer(PrimitiveData);

    //    DrawPrimitive(PrimitiveData);
    //}

    MRenderPass::RenderPass(PrimitiveDatas);
}

std::vector<FPrimitiveData> MFullScreenQuadPass::MakePrimitiveDatas()
{
    std::vector<FPrimitiveData> PrimitiveDatas;
    PrimitiveDatas.push_back(CreatePrimitiveData(EPrimitiveType::Mesh));

    return PrimitiveDatas;
}

FPrimitiveData MFullScreenQuadPass::CreatePrimitiveData(EPrimitiveType InType)
{
    FPrimitiveData NewPrimitivData = {};
    NewPrimitivData.MeshData = &MeshData;
    NewPrimitivData.PrimitiveType = InType;
    NewPrimitivData.ProjectionType = EProjectionType::Orthograhpic;

    FMeshBufferContainer BufferContainer = {};
    getGraphicDevice()->GetBuffers(BufferContainer, TEXT("Plane"));
    NewPrimitivData.VertexBuffer = BufferContainer.VertexBuffers[0];
    NewPrimitivData.IndexBuffer = BufferContainer.IndexBuffers[0];

    auto& Window = getRenderer()->GetCurrentScene()->GetWindow();
    NewPrimitivData.Scale.y = Window->GetHeight<float>();
    NewPrimitivData.Scale.x = Window->GetWidth<float>();

    return NewPrimitivData;
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
