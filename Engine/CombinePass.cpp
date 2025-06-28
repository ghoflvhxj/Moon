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
#include "MainGame.h"

// Actor
#include "Camera.h"

// 임시
#include "LightComponent.h"
#include "PointLightComponent.h"

#undef max
#undef min

using namespace DirectX;

CombinePass::CombinePass()
    : MRenderPass()
{
    bWriteDepthStencil = false;
}

void CombinePass::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));
}

void CombinePass::HandleOuputMergeStage(const FPrimitiveData& PrimitiveData)
{
    // DepthStencilState
    g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::EDepthWriteMode::Disable), 1);

    // OutputMergeState
    g_pGraphicDevice->getContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

bool GeometryPass::IsValidPrimitive(const FPrimitiveData &PrimitiveData) const
{
	return PrimitiveData.PrimitiveType == EPrimitiveType::Mesh && MRenderPass::IsValidPrimitive(PrimitiveData);
}

DirectionalShadowDepthPass::DirectionalShadowDepthPass()
	: MRenderPass()
    //, LightPosition(3)
    //, LightViewProj(3)
{
	SetUseOwningDepthStencilBuffer(ERenderTarget::DirectionalShadowDepth);
}

void DirectionalShadowDepthPass::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));
}

bool DirectionalShadowDepthPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    return PrimitiveData.PrimitiveType == EPrimitiveType::Mesh && MRenderPass::IsValidPrimitive(PrimitiveData);
}

void DirectionalShadowDepthPass::UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    MRenderPass::UpdateObjectConstantBuffer(PrimitiveData);

    // 패스 자체 쉐이더를 사용하기 때문에 오브젝트 Cbuffer를 수동으로 갱신해줌 (WorldMatrix, AnimMatrix 등등을 복사하는 것)
    auto& variableInfosVS = PrimitiveData.Material.lock()->getConstantBufferVariables(ShaderType::Vertex, EConstantBufferLayer::Object);
    _vertexShader->UpdateConstantBuffer(EConstantBufferLayer::Object, variableInfosVS);
}

PointShadowDepthPass::PointShadowDepthPass()
    : MRenderPass()
{
    SetUseOwningDepthStencilBuffer(ERenderTarget::PointShadowDepth);
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
        const std::shared_ptr<MLightComponent>& LightComponent = PrimitiveData.GetPrimitiveComponent<MLightComponent>();

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

void PointShadowDepthPass::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));
}

bool PointShadowDepthPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    return PrimitiveData.PrimitiveType == EPrimitiveType::Mesh && MRenderPass::IsValidPrimitive(PrimitiveData);
}

void PointShadowDepthPass::UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    MRenderPass::UpdateObjectConstantBuffer(PrimitiveData);

    std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock();

    // 패스 자체 쉐이더를 사용하기 때문에 오브젝트 Cbuffer로 수동으로 갱신해줌
    auto& variableInfosVS = Material->getConstantBufferVariables(ShaderType::Vertex, EConstantBufferLayer::Object);
    _vertexShader->UpdateConstantBuffer(EConstantBufferLayer::Object, variableInfosVS);
}

void DirectionalLightPass::UpdateObjectConstantBuffer(const FPrimitiveData &PrimitiveData)
{
	MPrimitiveComponent* PrimitiveComponent = PrimitiveData.PrimitiveComponent.lock().get();
    std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock();

	Vec3 trans = PrimitiveComponent->getWorldTranslation();
	Vec4 transAndRange = { trans.x, trans.y, trans.z, 10.f };
	Vec4 color = { 1.f, 1.f, 1.f, 1.f };
	if (std::shared_ptr<MLightComponent> LightComp = std::static_pointer_cast<MLightComponent>(PrimitiveData.PrimitiveComponent.lock()))
	{
		color.x = LightComp->getColor().x;
		color.y = LightComp->getColor().y;
		color.z = LightComp->getColor().z;
	}
	
	XMVECTOR rotationVector = XMLoadFloat3(&PrimitiveComponent->getRotation());
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotationVector);
	Mat4 rotMatrix = IDENTITYMATRIX;
	XMStoreFloat4x4(&rotMatrix, rotationMatrix);
	Vec3 look = { rotMatrix._31, rotMatrix._32, rotMatrix._33 };

	Material->getPixelShader()->SetValue(TEXT("g_lightPosition"), transAndRange);
	Material->getPixelShader()->SetValue(TEXT("g_lightDirection"), look);
	Material->getPixelShader()->SetValue(TEXT("g_lightColor"), color);

	Material->getPixelShader()->SetValue(TEXT("g_inverseCameraViewMatrix"), g_pMainGame->getMainCamera()->getInvesrViewMatrix());
	Material->getPixelShader()->SetValue(TEXT("g_inverseProjectiveMatrix"), g_pMainGame->getMainCamera()->getInversePerspectiveProjectionMatrix());

	MRenderPass::UpdateObjectConstantBuffer(PrimitiveData);
}

bool DirectionalLightPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    return PrimitiveData.PrimitiveType == EPrimitiveType::DirectionalLight && MRenderPass::IsValidPrimitive(PrimitiveData);
}

void DirectionalLightPass::HandleRasterizerStage(const FPrimitiveData& primitiveData)
{
    g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));
}

void DirectionalLightPass::HandleOuputMergeStage(const FPrimitiveData& primitiveData)
{
    g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::EDepthWriteMode::Disable), 1);
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

void SkyPass::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Frontface));
}

PointLightPass::PointLightPass()
    : MRenderPass()
{
    //SetClearTargets(false);
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
    std::shared_ptr<PointLightComponent> LightComp = PrimitiveData.GetPrimitiveComponent<PointLightComponent>();
    std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock();

    Vec3 trans = LightComp->getWorldTranslation();
    Vec4 transAndRange = { trans.x, trans.y, trans.z, LightComp->getRange()};
    Vec4 color = { 1.f, 1.f, 1.f, 1.f };
    color.x = LightComp->getColor().x;
    color.y = LightComp->getColor().y;
    color.z = LightComp->getColor().z;
    color.w = LightComp->getIntensity();

    Material->getPixelShader()->SetValue(TEXT("g_lightPosition"), transAndRange);
    Material->getPixelShader()->SetValue(TEXT("g_lightColor"), color);

    Material->getPixelShader()->SetValue(TEXT("PointLightIndex"), PointLightIndex++);

    Material->getPixelShader()->SetValue(TEXT("g_inverseCameraViewMatrix"), g_pMainGame->getMainCamera()->getInvesrViewMatrix());
    Material->getPixelShader()->SetValue(TEXT("g_inverseProjectiveMatrix"), g_pMainGame->getMainCamera()->getInversePerspectiveProjectionMatrix());

    MRenderPass::UpdateObjectConstantBuffer(PrimitiveData);
}

void PointLightPass::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));
}

void PointLightPass::HandleOuputMergeStage(const FPrimitiveData& primitiveData)
{
    g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::EDepthWriteMode::Disable), 1);
    g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Light), nullptr, 0xffffffff);
}

bool CollisionPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    return g_pRenderer->IsDrawCollision() && PrimitiveData.PrimitiveType == EPrimitiveType::Collision && MRenderPass::IsValidPrimitive(PrimitiveData);
}