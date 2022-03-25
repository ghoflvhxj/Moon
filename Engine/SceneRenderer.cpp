#include "stdafx.h"
#include "SceneRenderer.h"

// Input
#include "DirectInput.h"

// Graphics
#include "GraphicDevice.h"
#include "Material.h"

#include "RenderPass.h"

// Shader
#include "ShaderManager.h"

// System
#include "Thread.h"

// Framework
#include "MainGame.h"
#include "MainGameSetting.h"

#include "PrimitiveComponent.h"
#include "LightComponent.h"
#include "MeshComponent.h"
#include "SkyComponent.h"
#include "CollisionShapeComponent.h"

#include "RenderTarget.h"

#include "CollisionRenderer.h"

SceneRenderer::SceneRenderer(std::shared_ptr<MeshComponent> pDeferredRenderMesh)
{
	//ASSERT_CONDITION_MSG(nullptr != pDeferredRenderMesh, TEXT("디퍼드 렌더링 대상의 메시가 nullptr입니다! 확인필요"));

	//initializeRenderTarget();
}

SceneRenderer::~SceneRenderer()
{

}



void SceneRenderer::initializeRenderTarget()
{
	//_renderTargetList.push_back(std::make_shared<RenderTarget>());
	//_renderTargetList.push_back(std::make_shared<RenderTarget>());
	//_renderTargetList.push_back(std::make_shared<RenderTarget>());
	//_renderTargetList.push_back(std::make_shared<RenderTarget>());

	//// 디버그용
	//float scale = _renderTargetList[0]->getMeshComponent()->getScale().y;
	//float start = scale + 100.f;
	//_renderTargetList[0]->getMeshComponent()->setTranslation(Vec3{ -400.f, start, 1.f });
	//_renderTargetList[1]->getMeshComponent()->setTranslation(Vec3{ -400.f, start - scale , 1.f });
	//_renderTargetList[2]->getMeshComponent()->setTranslation(Vec3{ -400.f, start - 2.f * scale , 1.f });
	//_renderTargetList[3]->getMeshComponent()->setTranslation(Vec3{ -400.f, start - 3.f * scale, 1.f });

	//_lightDiffuseTarget = std::make_shared<RenderTarget>();
	//_lightDiffuseTarget->getMeshComponent()->setTranslation(Vec3{ -100.f, start, 1.f });
	//_lightSpecularTarget = std::make_shared<RenderTarget>();
	//_lightSpecularTarget->getMeshComponent()->setTranslation(Vec3{ -100.f, start - scale, 1.f });
}

void SceneRenderer::render()
{
	renderObject();
	//renderSky();
	//renderCollision();

	_primitiveComponentList.clear();
	_lightComponentList.clear();
	_skyComponentList.clear();
	_collisionComponentList.clear();
}

void SceneRenderer::renderObject()
{
	//fowardRender();
	deferredRender();
}

void SceneRenderer::fowardRender()
{
	//g_pGraphicDevice->getContext()->VSSetShader(g_pShaderManager->getVertexShader(TEXT("TexVertexShader.cso")), nullptr, 0);
	//g_pGraphicDevice->getContext()->PSSetShader(g_pShaderManager->getPixelShader(TEXT("TexPixelShader.cso")), nullptr, 0);

	for (auto iter = _primitiveComponentList.begin(); iter != _primitiveComponentList.end(); ++iter)
		(*iter)->render();
}

void SceneRenderer::deferredRender()
{
	renderAlbedo();
	//renderLight();
}

void renderPass()
{

}

void SceneRenderer::renderAlbedo()
{
	//ID3D11RenderTargetView *pRenderTargetView = nullptr;
	//ID3D11DepthStencilView *pDepthStencilView = nullptr;

	//g_pGraphicDevice->getContext()->OMGetRenderTargets(1, &pRenderTargetView, &pDepthStencilView);
	////-------------------------------------------------------------------------------------------------------------------------------------------------------------

	//size_t renderTargetCount = _renderTargetList.size();
	//std::vector<ID3D11RenderTargetView*> temp;
	//temp.reserve(_renderTargetList.size());

	//for (size_t i = 0; i < renderTargetCount; ++i)
	//{
	//	g_pGraphicDevice->getContext()->ClearRenderTargetView(_renderTargetList[i]->getRenderTargetView(), reinterpret_cast<const float *>(&Colors::Black));
	//	g_pGraphicDevice->getContext()->ClearDepthStencilView(_renderTargetList[i]->getDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
	//	temp.push_back(_renderTargetList[i]->getRenderTargetView());
	//}

	//g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(renderTargetCount), &temp[0], pDepthStencilView);

	//for (auto iter = _primitiveComponentList.begin(); iter != _primitiveComponentList.end(); ++iter)
	//	(*iter)->render();

	//ID3D11RenderTargetView *a[] = { nullptr, nullptr, nullptr, nullptr };
	//g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(renderTargetCount), a, nullptr);
	////-------------------------------------------------------------------------------------------------------------------------------------------------------------

	//g_pGraphicDevice->getContext()->OMSetRenderTargets(1u, &pRenderTargetView, pDepthStencilView);
}

void SceneRenderer::renderLight()
{
	//ID3D11RenderTargetView *pRenderTargetView = nullptr;
	//ID3D11DepthStencilView *pDepthStencilView = nullptr;

	//g_pGraphicDevice->getContext()->OMGetRenderTargets(1, &pRenderTargetView, &pDepthStencilView);
	////-------------------------------------------------------------------------------------------------------------------------------------------------------------

	//g_pGraphicDevice->getContext()->ClearRenderTargetView(_lightDiffuseTarget->getRenderTargetView(), reinterpret_cast<const float *>(&Colors::Black));
	//g_pGraphicDevice->getContext()->ClearRenderTargetView(_lightSpecularTarget->getRenderTargetView(), reinterpret_cast<const float *>(&Colors::Black));
	////g_pGraphicDevice->getContext()->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);

	//ID3D11RenderTargetView* temp[] = {
	//	_lightDiffuseTarget->getRenderTargetView(),
	//	_lightSpecularTarget->getRenderTargetView()
	//};

	//g_pGraphicDevice->getContext()->OMSetRenderTargets(2u, temp, nullptr);

	//ID3D11ShaderResourceView *pDepth	= _renderTargetList[1]->getShaderResouceView();
	//ID3D11ShaderResourceView *pNormal	= _renderTargetList[2]->getShaderResouceView();
	//ID3D11ShaderResourceView *pSpecular = _renderTargetList[3]->getShaderResouceView();

	//g_pGraphicDevice->getContext()->PSSetShaderResources(0u, 1u, &pDepth);
	//g_pGraphicDevice->getContext()->PSSetShaderResources(1u, 1u, &pNormal);
	//g_pGraphicDevice->getContext()->PSSetShaderResources(2u, 1u, &pSpecular);

	//for (auto iter = _lightComponentList.begin(); iter != _lightComponentList.end(); ++iter)
	//	(*iter)->render();

	//ID3D11RenderTargetView* a[] = { nullptr, nullptr };
	//g_pGraphicDevice->getContext()->OMSetRenderTargets(2u, a, nullptr);

	//ID3D11ShaderResourceView *p = nullptr;
	//g_pGraphicDevice->getContext()->PSSetShaderResources(0u, 1u, &p);
	//g_pGraphicDevice->getContext()->PSSetShaderResources(1u, 1u, &p);
	//g_pGraphicDevice->getContext()->PSSetShaderResources(2u, 1u, &p);
	////-------------------------------------------------------------------------------------------------------------------------------------------------------------

	//g_pGraphicDevice->getContext()->OMSetRenderTargets(1u, &pRenderTargetView, pDepthStencilView);
}

void SceneRenderer::renderSky()
{
	//ID3D11RenderTargetView *pRenderTargetView = nullptr;
	//ID3D11DepthStencilView *pDepthStencilView = nullptr;

	//g_pGraphicDevice->getContext()->OMGetRenderTargets(1, &pRenderTargetView, &pDepthStencilView);

	////-------------------------------------------------------------------------------------------------------------------------------------------------------------
	//ID3D11ShaderResourceView *pDepth = _renderTargetList[1]->getShaderResouceView();
	//g_pGraphicDevice->getContext()->PSSetShaderResources(1u, 1u, &pDepth);

	//ID3D11RenderTargetView *temp[] = {
	//	_renderTargetList[0]->getRenderTargetView(),
	//	_lightDiffuseTarget->getRenderTargetView(),
	//};

	//g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(std::size(temp)), temp, nullptr);

	//for (auto iter = _skyComponentList.begin(); iter != _skyComponentList.end(); ++iter)
	//{
	//	(*iter)->Test(pDepth);
	//	(*iter)->render();
	//}

	//ID3D11RenderTargetView* a[] = { nullptr, nullptr };
	//g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(std::size(a)), a, nullptr);

	//ID3D11ShaderResourceView *p = nullptr;
	//g_pGraphicDevice->getContext()->PSSetShaderResources(0u, 1u, &p);
	//g_pGraphicDevice->getContext()->PSSetShaderResources(1u, 1u, &p);

	////-------------------------------------------------------------------------------------------------------------------------------------------------------------
	//g_pGraphicDevice->getContext()->OMSetRenderTargets(1u, &pRenderTargetView, pDepthStencilView);
}

void SceneRenderer::renderCollision()
{
	//ID3D11RenderTargetView *pRenderTargetView = nullptr;
	//ID3D11DepthStencilView *pDepthStencilView = nullptr;

	//g_pGraphicDevice->getContext()->OMGetRenderTargets(1, &pRenderTargetView, &pDepthStencilView);

	////-------------------------------------------------------------------------------------------------------------------------------------------------------------
	//ID3D11RenderTargetView *temp[] = {
	//	_renderTargetList[0]->getRenderTargetView(),
	//	_renderTargetList[1]->getRenderTargetView(),
	//	_lightDiffuseTarget->getRenderTargetView(),
	//};

	//g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(std::size(temp)), temp, pDepthStencilView);

	//for (auto iter = _collisionComponentList.begin(); iter != _collisionComponentList.end(); ++iter)
	//{
	//	(*iter)->render();
	//}

	//ID3D11RenderTargetView *a[] = { nullptr, nullptr, nullptr };
	//g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(std::size(a)), a, nullptr);

	////-------------------------------------------------------------------------------------------------------------------------------------------------------------
	//g_pGraphicDevice->getContext()->OMSetRenderTargets(1u, &pRenderTargetView, pDepthStencilView);
}


unsigned int SceneRenderer::RenderFunc(void *pParam)
{
	if (pParam == nullptr)
		return 0;

	PrimitiveComponentList componentList(*static_cast<PrimitiveComponentList *>(pParam));

	for (auto iter = componentList.begin(); iter != componentList.end(); ++iter)
		(*iter)->render();

	ID3D11CommandList *pCommandList = nullptr;
	FAILED_CHECK_THROW(g_pGraphicDevice->getDefferedContext()->FinishCommandList(FALSE, &pCommandList));
	g_pGraphicDevice->getDefferedContext()->ClearState();

	if (nullptr != pCommandList)
	{
		g_pGraphicDevice->getImmediateContext()->ExecuteCommandList(pCommandList, FALSE);
		pCommandList->Release();
	}

	return 0;
}

void SceneRenderer::addPrimitiveComponent(std::shared_ptr<PrimitiveComponent> pComponent)
{
	_primitiveComponentList.push_back(pComponent);
}

void SceneRenderer::addLightComponent(std::shared_ptr<LightComponent> pComponent)
{
	_lightComponentList.push_back(pComponent);
}

void SceneRenderer::addSkyComponent(std::shared_ptr<SkyComponent> pComponent)
{
	_skyComponentList.push_back(pComponent);
}

void SceneRenderer::addCollisionShapeComponent(std::shared_ptr<PrimitiveComponent> pComponent)
{
	_collisionComponentList.push_back(std::static_pointer_cast<CollisionShapeComponent>(pComponent));
}
