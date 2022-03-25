#include "stdafx.h"
#include "Renderer.h"

#include "GraphicDevice.h"
#include "Material.h"

#include "RenderTarget.h"
#include "RenderPass.h"

#include "SceneRenderer.h"
#include "CollisionRenderer.h"

// Framework
#include "MainGame.h"
#include "MainGameSetting.h"

#include "PrimitiveComponent.h"
#include "LightComponent.h"
#include "MeshComponent.h"

#include "MapUtility.h"

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
	_pMeshComponent->getMaterial()->setShader(TEXT("TexVertexShader.cso"), TEXT("DeferredShader.cso"));

	// xml 로딩으로 대체하기
	addRenderTarget(TEXT("Albedo"));
	addRenderTarget(TEXT("Depth"));
	addRenderTarget(TEXT("Normal"));
	addRenderTarget(TEXT("Specular"));

	addRenderTarget(TEXT("LightDiffuse"));
	addRenderTarget(TEXT("LightSpecular"));

	addRenderPass(TEXT("Mesh"));
	{
		std::vector<std::shared_ptr<RenderTarget>> rt = {
			_renderTargetMap[TEXT("Albedo")],
			_renderTargetMap[TEXT("Depth")],
			_renderTargetMap[TEXT("Normal")],
			_renderTargetMap[TEXT("Specular")]
		};
		_renderPassMap[TEXT("Mesh")]->initializeRenderTarget(rt);
	}

	addRenderPass(TEXT("Light"));
	{
		std::vector<std::shared_ptr<RenderTarget>> rt = {
			_renderTargetMap[TEXT("LightDiffuse")],
			_renderTargetMap[TEXT("LightSpecular")],
		};

		std::vector<std::shared_ptr<TextureComponent>> textureList = {
			_renderTargetMap[TEXT("Depth")]->getRenderTargetTexture(),
			_renderTargetMap[TEXT("Normal")]->getRenderTargetTexture(),
			_renderTargetMap[TEXT("Specular")]->getRenderTargetTexture()
		};

		_renderPassMap[TEXT("Light")]->initializeRenderTarget(rt);
		_renderPassMap[TEXT("Light")]->initializeTexture(textureList);
	}

	_pMeshComponent->setTexture(int32ToEnum<TextureType>(0), _renderTargetMap[TEXT("Albedo")]->getRenderTargetTexture());
	_pMeshComponent->setTexture(int32ToEnum<TextureType>(1), _renderTargetMap[TEXT("LightDiffuse")]->getRenderTargetTexture());
	_pMeshComponent->setTexture(int32ToEnum<TextureType>(2), _renderTargetMap[TEXT("LightSpecular")]->getRenderTargetTexture());

	_pSceneRenderer		= std::make_shared<SceneRenderer>(_pMeshComponent);
	//_pCollisionRenderer = std::make_shared<CollisionRenderer>();
}

const bool Renderer::addRenderTarget(const std::wstring name)
{
	bool result = MapUtility::FindInsert(_renderTargetMap, name, std::make_shared<RenderTarget>());

#ifdef _DEBUG
	if (true == result)
	{
		std::shared_ptr<MeshComponent> pMeshComponent = std::make_shared<MeshComponent>();
		result = MapUtility::FindInsert(_renderTargetMeshMap, name, pMeshComponent);
		if (true == result)
		{
			float scale = 200.f;
			uint32 count = _renderTargetMeshMap.size() - 1;

			pMeshComponent->setScale(scale, scale, 0.f);
			pMeshComponent->setTranslation(scale / 2.f + (count / 4 * scale), scale / 2.f + (count * scale), 0.f);
		}
	}
#endif

	return result;
}

const bool Renderer::addRenderPass(const std::wstring name)
{
	return MapUtility::FindInsert(_renderPassMap, name, std::make_shared<RenderPass>());
}

void Renderer::render(void)
{
	g_pGraphicDevice->Begin();

	//-------------------------------------------------------------------------------------------------------------------------------------------------------------
	// 주요 렌더링
	_renderPassMap[TEXT("Mesh")]->begin();
	for (auto iter = _pSceneRenderer->_primitiveComponentList.begin(); iter != _pSceneRenderer->_primitiveComponentList.end(); ++iter)
		(*iter)->render();
	_pSceneRenderer->_primitiveComponentList.clear();
	_renderPassMap[TEXT("Mesh")]->end();

	_renderPassMap[TEXT("Light")]->begin();
	for (auto iter = _pSceneRenderer->_lightComponentList.begin(); iter != _pSceneRenderer->_lightComponentList.end(); ++iter)
		(*iter)->render();
	_pSceneRenderer->_lightComponentList.clear();
	_renderPassMap[TEXT("Light")]->end();

	//_pSceneRenderer->render();
	//_renderPassMap[TEXT("Deferred")]->begin();
	_pMeshComponent->render();
	//_renderPassMap[TEXT("Deferred")]->end();
	//_pCollisionRenderer->render();

	//-------------------------------------------------------------------------------------------------------------------------------------------------------------
	// 메인 게임의 렌더링, 주로 ImGUI가 출력됨
	g_pMainGame->render();

	//-------------------------------------------------------------------------------------------------------------------------------------------------------------
	// 렌더 타겟
#ifdef _DEBUG
	if (true == _drawRenderTarget)
	{
		for each (auto pair in _renderTargetMeshMap)
		{
			pair.second->render();
		}
	}
#endif

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
