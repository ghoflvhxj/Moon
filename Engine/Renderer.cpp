#include "stdafx.h"
#include "Renderer.h"

#include "GraphicDevice.h"
#include "Material.h"

#include "MainGame.h"

#include "SceneRenderer.h"
#include "CollisionRenderer.h"

// Framework
#include "MainGame.h"
#include "MainGameSetting.h"

#include "PrimitiveComponent.h"
#include "MeshComponent.h"

Renderer::Renderer(void) noexcept
	: _pSceneRenderer{ nullptr }
	, _pCollisionRenderer{ nullptr }
	, _drawRenderTarget{ true }
{
	initialize();
}

Renderer::~Renderer() noexcept
{

}

void Renderer::initialize(void) noexcept
{
	_pMeshComponent = std::make_shared<MeshComponent>();
	_pMeshComponent->setRenderMode(PrimitiveComponent::RenderMode::Orthogonal);
	_pMeshComponent->setTranslation(Vec3{ 0.f, 0.f, 2.f });
	_pMeshComponent->setScale(Vec3{ static_cast<float>(g_pSetting->getResolutionWidth()), static_cast<float>(g_pSetting->getResolutionHeight()), 1.f });
	_pMeshComponent->SceneComponent::Update(0.f);
	_pMeshComponent->getMaterial()->setDepthWriteMode(Graphic::DepthWriteMode::Disable);

	_pSceneRenderer		= std::make_shared<SceneRenderer>(_pMeshComponent);
	//_pCollisionRenderer = std::make_shared<CollisionRenderer>();
}

void Renderer::render(void) const noexcept
{
	g_pGraphicDevice->Begin();

	//-------------------------------------------------------------------------------------------------------------------------------------------------------------
	// 주요 렌더링
	_pSceneRenderer->render();
	_pMeshComponent->render();

	//_pCollisionRenderer->render();

	if (true == _drawRenderTarget)
	{
		_pSceneRenderer->renderRenderTargets();
	}
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------
	// 메인 게임의 렌더링, 주로 ImGUI가 출력됨
	g_pMainGame->render();

	g_pGraphicDevice->End();
}

void Renderer::addPrimitiveComponent(std::shared_ptr<PrimitiveComponent> pComponent) noexcept
{
	_pSceneRenderer->addPrimitiveComponent(pComponent);
}

void Renderer::addLightComponent(std::shared_ptr<LightComponent> pComponent) noexcept
{
	_pSceneRenderer->addLightComponent(pComponent);
}

void Renderer::addSkyComponent(std::shared_ptr<SkyComponent> pComponent) noexcept
{
	_pSceneRenderer->addSkyComponent(pComponent);
}

void Renderer::addCollisionShapeComponent(std::shared_ptr<PrimitiveComponent> pComponent) noexcept
{
	_pSceneRenderer->addCollisionShapeComponent(pComponent);
}

void Renderer::toggleRenderTarget()
{
	_drawRenderTarget = (true == _drawRenderTarget) ? false : true;
}
