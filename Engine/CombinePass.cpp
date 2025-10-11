#include "Include.h"
#include "CombinePass.h"

#include "MainGameSetting.h"

// Renderer
#include "Renderer.h"

// Graphic
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"

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

bool GeometryPass::IsValidPrimitive(const FPrimitiveData &PrimitiveData) const
{
	return PrimitiveData.PrimitiveType == EPrimitiveType::Mesh && MRenderPass::IsValidPrimitive(PrimitiveData);
}

DirectionalShadowDepthPass::DirectionalShadowDepthPass()
	: MRenderPass()
{
	SetUseOwningDepthStencilBuffer(ERenderTarget::DirectionalShadowDepth);
    bUseDefaultShaderOnly = true;
}

bool DirectionalShadowDepthPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    if (MRenderPass::IsValidPrimitive(PrimitiveData))
    {
        return PrimitiveData.PrimitiveType == EPrimitiveType::Mesh && PrimitiveData.PrimitiveComponent.lock()->IsShadowing();
    }

    return false;
}

PointShadowDepthPass::PointShadowDepthPass()
    : MRenderPass()
{
    SetUseOwningDepthStencilBuffer(ERenderTarget::PointShadowDepth);
    bUseDefaultShaderOnly = true;
}

void PointShadowDepthPass::RenderPass(const std::vector<FPrimitiveData>& PrimitiveDatList)
{
    Begin();

    auto& PointLightPrimitives = g_pRenderer->GetPrimitives(EPrimitiveType::PointLight);
    auto& MeshPrimitives = g_pRenderer->GetPrimitives(EPrimitiveType::Mesh);
    uint32 PointLightNum = GetSize(PointLightPrimitives);

    for (uint32 PointLightIndex = 0; PointLightIndex < PointLightNum; ++PointLightIndex)
    {
        const FPrimitiveData& PrimitiveData = PointLightPrimitives[PointLightIndex];
        std::shared_ptr<MLightComponent>& LightComponent = PrimitiveData.GetPrimitiveComponent<MLightComponent>();

        // 콘스탄트 버퍼 업데이트
        Vec3 Position = LightComponent->getTranslation();
        _geometryShader->SetValue(TEXT("PointLightPos"), Position);
        _geometryShader->SetValue(TEXT("PointLightIndex"), PointLightIndex);

        XMVECTOR Up = XMLoadFloat3(&VEC3UP);
        XMMATRIX Proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), 1.f, 0.1f, 1000.f);
        XMVECTOR LoadedPosition = XMLoadFloat3(&Position);

        std::vector<Mat4> PointLightViewProj(6);
        XMStoreFloat4x4(&PointLightViewProj[0], XMMatrixLookAtLH(LoadedPosition, LoadedPosition + XMVectorSet(1.f, 0.f, 0.f, 0.f), Up) * Proj);
        XMStoreFloat4x4(&PointLightViewProj[1], XMMatrixLookAtLH(LoadedPosition, LoadedPosition + XMVectorSet(-1.f, 0.f, 0.f, 0.f), Up) * Proj);
        XMStoreFloat4x4(&PointLightViewProj[2], XMMatrixLookAtLH(LoadedPosition, LoadedPosition + XMVectorSet(0.f, 1.f, 0.f, 0.f), XMVectorSet(0.f, 0.f, 1.f, 1.f)) * Proj);
        XMStoreFloat4x4(&PointLightViewProj[3], XMMatrixLookAtLH(LoadedPosition, LoadedPosition + XMVectorSet(0.f, -1.f, 0.f, 0.f), XMVectorSet(0.f, 0.f, 1.f, 1.f)) * Proj);
        XMStoreFloat4x4(&PointLightViewProj[4], XMMatrixLookAtLH(LoadedPosition, LoadedPosition + XMVectorSet(0.f, 0.f, 1.f, 0.f), Up) * Proj);
        XMStoreFloat4x4(&PointLightViewProj[5], XMMatrixLookAtLH(LoadedPosition, LoadedPosition + XMVectorSet(0.f, 0.f, -1.f, 0.f), Up) * Proj);
        _geometryShader->SetValue(TEXT("PointLightViewProj"), PointLightViewProj);

        for (const FPrimitiveData& MeshPrimitiveData : MeshPrimitives)
        {
            UpdateObjectConstantBuffer(MeshPrimitiveData);
            DrawPrimitive(MeshPrimitiveData);
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

void DirectionalLightPass::UpdateObjectConstantBuffer(const FPrimitiveData &PrimitiveData)
{
	auto& PrimitiveComponent = PrimitiveData.PrimitiveComponent.lock()->CastTo<MDirectionalLightComponent>();
    std::shared_ptr<MShader>& PixelShader = GetPixelShader(PrimitiveData);

	Vec3 trans = PrimitiveComponent->getWorldTranslation();
	Vec4 transAndRange = { trans.x, trans.y, trans.z, 10.f };
	Vec4 color = { 1.f, 1.f, 1.f, 1.f };
	if (std::shared_ptr<MLightComponent> LightComp = std::static_pointer_cast<MLightComponent>(PrimitiveData.PrimitiveComponent.lock()))
	{
		color.x = LightComp->getColor().x;
		color.y = LightComp->getColor().y;
		color.z = LightComp->getColor().z;
	}
	
    const Vec3& Direction = PrimitiveComponent->GetDirection();

	PixelShader->SetValue(TEXT("g_lightPosition"), transAndRange);
	PixelShader->SetValue(TEXT("g_lightDirection"), Direction);
	PixelShader->SetValue(TEXT("g_lightColor"), color);
	PixelShader->SetValue(TEXT("g_inverseCameraViewMatrix"), g_World->getMainCamera()->getInvesrViewMatrix());
	PixelShader->SetValue(TEXT("g_inverseProjectiveMatrix"), g_World->getMainCamera()->getInversePerspectiveProjectionMatrix());

	MRenderPass::UpdateObjectConstantBuffer(PrimitiveData);
}

bool DirectionalLightPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    return PrimitiveData.PrimitiveType == EPrimitiveType::DirectionalLight && MRenderPass::IsValidPrimitive(PrimitiveData);
}

void DirectionalLightPass::HandleOutputMergeStage(const FPrimitiveData& primitiveData)
{
    uint32 DepthStencilFlag = 0;
    DepthStencilFlag |= (uint32)Graphic::EDepthStencilMode::DepthDisable;
    DepthStencilFlag |= (uint32)Graphic::EDepthStencilMode::StencilDisable;

    g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(DepthStencilFlag), 0);
    g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Light), nullptr, 0xffffffff);
}

bool SkyPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
	if (PrimitiveData.PrimitiveType != EPrimitiveType::Sky)
	{
		return false;
	}

	return MRenderPass::IsValidPrimitive(PrimitiveData);
}

void PointLightPass::End()
{
    MRenderPass::End();
    PointLightIndex = 0;
}

bool PointLightPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    //return false;
    return PrimitiveData.PrimitiveType == EPrimitiveType::PointLight && MRenderPass::IsValidPrimitive(PrimitiveData);
}

void PointLightPass::UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    std::shared_ptr<MPointLightComponent> LightComp = PrimitiveData.GetPrimitiveComponent<MPointLightComponent>();
    std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock();

    Vec3 trans = LightComp->getWorldTranslation();
    Vec4 transAndRange = { trans.x, trans.y, trans.z, LightComp->getRange()};
    Vec4 color = { 1.f, 1.f, 1.f, 1.f };
    color.x = LightComp->getColor().x;
    color.y = LightComp->getColor().y;
    color.z = LightComp->getColor().z;
    color.w = LightComp->getIntensity();

    if (std::shared_ptr<MShader>& PixelShader = Material->getPixelShader())
    {
        PixelShader->SetValue(TEXT("g_lightPosition"), transAndRange);
        PixelShader->SetValue(TEXT("g_lightColor"), color);

        PixelShader->SetValue(TEXT("PointLightIndex"), PointLightIndex);

        PixelShader->SetValue(TEXT("g_inverseCameraViewMatrix"), g_World->getMainCamera()->getInvesrViewMatrix());
        PixelShader->SetValue(TEXT("g_inverseProjectiveMatrix"), g_World->getMainCamera()->getInversePerspectiveProjectionMatrix());

        Mat4 Mat = {};
        XMMATRIX XMMat = XMLoadFloat4x4(&g_World->getMainCamera()->getInversePerspectiveProjectionMatrix()) * XMLoadFloat4x4(&g_World->getMainCamera()->getInvesrViewMatrix());
        XMStoreFloat4x4(&Mat, XMMat);
        PixelShader->SetValue(TEXT("ScreenToWorldMatrix"), Mat);
    }



    MRenderPass::UpdateObjectConstantBuffer(PrimitiveData);
}

void PointLightPass::HandleOutputMergeStage(const FPrimitiveData& primitiveData)
{
    uint32 DepthStencilFlag = 0;
    DepthStencilFlag |= (uint32)Graphic::EDepthStencilMode::DepthDisable;
    DepthStencilFlag |= (uint32)Graphic::EDepthStencilMode::StencilDisable;

    g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(DepthStencilFlag), 0);
    g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Light), nullptr, 0xffffffff);
}

MLinePass::MLinePass()
    : MRenderPass()
{
    DefaultTopology = D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
}

bool MLinePass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    return g_pRenderer->IsDrawCollision() && PrimitiveData.PrimitiveType == EPrimitiveType::Collision && MRenderPass::IsValidPrimitive(PrimitiveData);
}

MEditorPass::MEditorPass()
    : MRenderPass()
{
    DefaultTopology = D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
}

bool MEditorPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    return MRenderPass::IsValidPrimitive(PrimitiveData);
}

MDepthPre::MDepthPre()
{
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
