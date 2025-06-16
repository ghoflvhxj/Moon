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

#undef max
#undef min

using namespace DirectX;

CombinePass::CombinePass()
    : RenderPass()
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
	return PrimitiveData._primitiveType == EPrimitiveType::Mesh && RenderPass::IsValidPrimitive(PrimitiveData);
}

DirectionalShadowDepthPass::DirectionalShadowDepthPass()
	: RenderPass()
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
    return PrimitiveData._primitiveType == EPrimitiveType::Mesh && RenderPass::IsValidPrimitive(PrimitiveData);
}

void DirectionalShadowDepthPass::UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    RenderPass::UpdateObjectConstantBuffer(PrimitiveData);

    // 패스 자체 쉐이더를 사용하기 때문에 오브젝트 Cbuffer를 수동으로 갱신해줌 (WorldMatrix, AnimMatrix 등등을 복사하는 것)
    auto& variableInfosVS = PrimitiveData._pMaterial.lock()->getConstantBufferVariables(ShaderType::Vertex, EConstantBufferLayer::Object);
    _vertexShader->UpdateConstantBuffer(EConstantBufferLayer::Object, variableInfosVS);
}

PointShadowDepthPass::PointShadowDepthPass()
    : RenderPass()
{
    SetUseOwningDepthStencilBuffer(ERenderTarget::PointShadowDepth);
}

void PointShadowDepthPass::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));
}

bool PointShadowDepthPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    return PrimitiveData._primitiveType == EPrimitiveType::Mesh && RenderPass::IsValidPrimitive(PrimitiveData);
}

void PointShadowDepthPass::UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    RenderPass::UpdateObjectConstantBuffer(PrimitiveData);

    // 패스 자체 쉐이더를 사용하기 때문에 오브젝트 Cbuffer를 수동으로 갱신해줌
    auto& variableInfosVS = PrimitiveData._pMaterial.lock()->getConstantBufferVariables(ShaderType::Vertex, EConstantBufferLayer::Object);
    _vertexShader->UpdateConstantBuffer(EConstantBufferLayer::Object, variableInfosVS);
}

void DirectionalLightPass::UpdateObjectConstantBuffer(const FPrimitiveData &PrimitiveData)
{
	PrimitiveComponent* PrimitiveComponent = PrimitiveData._pPrimitive.lock().get();

	Vec3 trans = PrimitiveComponent->getWorldTranslation();
	Vec4 transAndRange = { trans.x, trans.y, trans.z, 10.f };
	Vec4 color = { 1.f, 1.f, 1.f, 1.f };
	if (std::shared_ptr<MLightComponent> LightComp = std::static_pointer_cast<MLightComponent>(PrimitiveData._pPrimitive.lock()))
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

	PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_lightPosition"), transAndRange);
	PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_lightDirection"), look);
	PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_lightColor"), color);
	PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_inverseCameraViewMatrix"), g_pMainGame->getMainCamera()->getInvesrViewMatrix());
	PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_inverseProjectiveMatrix"), g_pMainGame->getMainCamera()->getInversePerspectiveProjectionMatrix());

	RenderPass::UpdateObjectConstantBuffer(PrimitiveData);
}

bool DirectionalLightPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    return PrimitiveData._primitiveType == EPrimitiveType::DirectionalLight && RenderPass::IsValidPrimitive(PrimitiveData);
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
	if (PrimitiveData._primitiveType != EPrimitiveType::Sky)
	{
		return false;
	}

	return RenderPass::IsValidPrimitive(PrimitiveData);
}

void SkyPass::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Frontface));
}

bool CollisionPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    return g_pRenderer->IsDrawCollision() && PrimitiveData._primitiveType == EPrimitiveType::Collision && RenderPass::IsValidPrimitive(PrimitiveData);
}

PointLightPass::PointLightPass()
    : RenderPass()
{
    //SetClearTargets(false);
}

bool PointLightPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    //return false;
    return PrimitiveData._primitiveType == EPrimitiveType::PointLight && RenderPass::IsValidPrimitive(PrimitiveData);
}

void PointLightPass::UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    PrimitiveComponent* PrimitiveComponent = PrimitiveData._pPrimitive.lock().get();

    Vec3 trans = PrimitiveComponent->getWorldTranslation();
    Vec4 transAndRange = { trans.x, trans.y, trans.z, 10.f };
    Vec4 color = { 1.f, 1.f, 1.f, 1.f };
    if (std::shared_ptr<MLightComponent> LightComp = std::static_pointer_cast<MLightComponent>(PrimitiveData._pPrimitive.lock()))
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

    PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_lightPosition"), transAndRange);
    PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_lightDirection"), look);
    PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_lightColor"), color);
    PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_inverseCameraViewMatrix"), g_pMainGame->getMainCamera()->getInvesrViewMatrix());
    PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_inverseProjectiveMatrix"), g_pMainGame->getMainCamera()->getInversePerspectiveProjectionMatrix());

    RenderPass::UpdateObjectConstantBuffer(PrimitiveData);
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
