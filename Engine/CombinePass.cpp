#include "CombinePass.h"
#include "MoonEngine.h"

#include "MainGameSetting.h"

// Renderer
#include "Renderer.h"

// Graphic
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"
#include "RenderTarget.h"

// Game
#include "World.h"

// Actor
#include "Camera.h"

// 임시
#include "LightComponent.h"
#include "DirectionalLightComponent.h"
#include "PointLightComponent.h"

#undef max
#undef min

using namespace DirectX;

GeometryPass::GeometryPass()
    : MRenderPass()
{
    UseCommonDepthStencil();
}

bool GeometryPass::IsValidPrimitive(const FPrimitiveData &PrimitiveData) const
{
	return PrimitiveData.PrimitiveType == EPrimitiveType::Mesh && MRenderPass::IsValidPrimitive(PrimitiveData);
}

DirectionalShadowDepthPass::DirectionalShadowDepthPass()
	: MRenderPass()
{
    bUseDefaultShaderOnly = true;
}

bool DirectionalShadowDepthPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    if (MRenderPass::IsValidPrimitive(PrimitiveData))
    {
        if (PrimitiveData.PrimitiveType == EPrimitiveType::Mesh)
        {
            if (PrimitiveData.PrimitiveComponent.lock())
            {
                return PrimitiveData.PrimitiveComponent.lock()->IsShadowing();
            }
        }
    }

    return false;
}

PointShadowDepthPass::PointShadowDepthPass()
    : MRenderPass()
{
    bUseDefaultShaderOnly = true;
    Color = { 1000.f, 1000.f, 1000.f, 1.f };
}

void PointShadowDepthPass::HandleOutputMergeStage(const FPrimitiveData& PrimitiveData)
{
    g_pGraphicDevice->LinearDepthStencil();
}

void PointShadowDepthPass::RenderPass(const std::vector<FPrimitiveData>& PrimitiveDatList)
{
    Begin();

    auto& PointLightPrimitives = g_pRenderer->GetPrimitiveDatas(EPrimitiveType::PointLight);
    auto& MeshPrimitives = g_pRenderer->GetPrimitiveDatas(EPrimitiveType::Mesh);
    uint32 PointLightNum = GetSize(PointLightPrimitives);

    for (uint32 PointLightIndex = 0; PointLightIndex < PointLightNum; ++PointLightIndex)
    {
        const FPrimitiveData& PrimitiveData = *PointLightPrimitives[PointLightIndex];
        std::shared_ptr<MPointLightComponent>& LightComponent = PrimitiveData.GetPrimitiveComponent<MPointLightComponent>();

        if (LightComponent->IsShadowing() == false)
        {
            continue;
        }

        // 콘스탄트 버퍼 업데이트
        Vec3 Position = LightComponent->getTranslation();
        _geometryShader->SetValue(TEXT("PointLightPos"), Position);
        _geometryShader->SetValue(TEXT("PointLightIndex"), PointLightIndex);

        XMVECTOR XMPosition = XMLoadFloat3(&Position);
        XMVECTOR Up = XMLoadFloat3(&VEC3UP);

        // 순서는 오른쪽, 왼쪽, 위, 아래, 앞, 뒤
        std::vector<Mat4> PointLightView(6);
        XMStoreFloat4x4(&PointLightView[0], XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(1.f, 0.f, 0.f, 0.f), Up));
        XMStoreFloat4x4(&PointLightView[1], XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(-1.f, 0.f, 0.f, 0.f), Up));
        XMStoreFloat4x4(&PointLightView[2], XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(0.f, 1.f, 0.f, 0.f), XMVectorSet(0.f, 0.f, 1.f, 0.f)));
        XMStoreFloat4x4(&PointLightView[3], XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(0.f, -1.f, 0.f, 0.f), XMVectorSet(0.f, 0.f, 1.f, 0.f)));
        XMStoreFloat4x4(&PointLightView[4], XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(0.f, 0.f, 1.f, 0.f), Up));
        XMStoreFloat4x4(&PointLightView[5], XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(0.f, 0.f, -1.f, 0.f), Up));

        float Near = GraphicDevice::bReverseDepth ? 1000.f : 0.1f;
        float Far = GraphicDevice::bReverseDepth ? 0.1f : 1000.f;
        XMMATRIX Proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), 1.f, Near, Far);
        std::vector<Mat4> PointLightViewProj(6);
        for (uint32 i = 0; i < 6; ++i)
        {
            XMStoreFloat4x4(&PointLightViewProj[i], XMLoadFloat4x4(&PointLightView[i]) * Proj);
        }
        _geometryShader->SetValue(TEXT("PointLightViewProj"), PointLightViewProj);

        for (const FPrimitiveData* MeshPrimitiveData : MeshPrimitives)
        {
            if (IsValidPrimitive(*MeshPrimitiveData) == false)
            {
                continue;
            }

            UpdateObjectConstantBuffer(*MeshPrimitiveData);
            DrawPrimitive(*MeshPrimitiveData);
        }
    }

    End();
}

bool PointShadowDepthPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    if (MRenderPass::IsValidPrimitive(PrimitiveData))
    {
        return PrimitiveData.PrimitiveType == EPrimitiveType::Mesh && PrimitiveData.PrimitiveComponent.lock()->IsShadowing();
    }

    return false;
}

bool SkyPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
	if (PrimitiveData.PrimitiveType != EPrimitiveType::Sky)
	{
		return false;
	}

	return MRenderPass::IsValidPrimitive(PrimitiveData);
}

MLinePass::MLinePass()
    : MRenderPass()
{
    DefaultTopology = D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
}

bool MLinePass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    return g_pRenderer->GetCurrentScene()->IsDrawCollision() && PrimitiveData.PrimitiveType == EPrimitiveType::Collision && MRenderPass::IsValidPrimitive(PrimitiveData);
}

MDepthPre::MDepthPre()
{
    UseCommonDepthStencil();
}

bool MDepthPre::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    if (MRenderPass::IsValidPrimitive(PrimitiveData))
    {
        bool bAlphaMasked = false;
        if (std::shared_ptr<MMaterial> Material = PrimitiveData.Material.lock())
        {
            bAlphaMasked = Material->IsAlphaMasked();
        }

        return PrimitiveData.PrimitiveType == EPrimitiveType::Mesh && bAlphaMasked == false;
    }

    return false;
}

void MDepthPre::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface, true));
}
