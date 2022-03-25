#include "stdafx.h"
#include "RenderPass.h"

#include "GraphicDevice.h"

#include "RenderTarget.h"

#include "TextureComponent.h"

RenderPass::RenderPass()
	: _renderTargetList()
	, _pOldRenderTargetView{ nullptr }
	, _pOldDepthStencilView{ nullptr }
{
	_renderTargetList.reserve(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
}

RenderPass::~RenderPass()
{
}

void RenderPass::begin()
{
	// ±âÁ¸ Á¤º¸ ÀúÀå
	g_pGraphicDevice->getContext()->OMGetRenderTargets(1, &_pOldRenderTargetView, &_pOldDepthStencilView);

	// ·»´õ Å¸°Ù ¼³Á¤
	uint32 renderTargetCount = sizeToUint32(_renderTargetList.size());
	std::vector<ID3D11RenderTargetView*> rowRenderTargetViewArray(renderTargetCount, nullptr);

	for (uint32 i = 0; i < renderTargetCount; ++i)
	{
		g_pGraphicDevice->getContext()->ClearRenderTargetView(_renderTargetList[i]->getRenderTargetView(), reinterpret_cast<const float *>(&Colors::Black));
		g_pGraphicDevice->getContext()->ClearDepthStencilView(_renderTargetList[i]->getDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
		rowRenderTargetViewArray[i] = _renderTargetList[i]->getRenderTargetView();
	}

	g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(renderTargetCount), &rowRenderTargetViewArray[0], _pOldDepthStencilView);

	// ½¦ÀÌ´õ ¸®¼Ò½º ºä ¼³Á¤
	uint32 textureCount = sizeToUint32(_textureList.size());
	for (uint32 i = 0; i < textureCount; ++i)
	{
		g_pGraphicDevice->getContext()->PSSetShaderResources(i, 1, &_textureList[i]->getResourceViewRowPointer());
	}
}

void RenderPass::end()
{
	// ½¦ÀÌ´õ ¸®¼Ò½º ºä ÇØÁ¦
	uint32 textureCount = sizeToUint32(_textureList.size());
	for (uint32 i = 0; i < textureCount; ++i)
	{
		ID3D11ShaderResourceView *pNullShaderResouceView = nullptr;
		g_pGraphicDevice->getContext()->PSSetShaderResources(i, 1, &pNullShaderResouceView);
	}

	// ·»´õ Å¸°Ù ÇØÁ¦¿Í ±âÁ¸ Á¤º¸ º¹±¸
	uint32 renderTargetCount = _renderTargetList.size();
	std::vector<ID3D11RenderTargetView*> restoreRenderTargetViewArray(renderTargetCount, nullptr);
	restoreRenderTargetViewArray[0] = _pOldRenderTargetView;
	g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(renderTargetCount), &restoreRenderTargetViewArray[0], _pOldDepthStencilView);
}

void RenderPass::initializeRenderTarget(std::vector<std::shared_ptr<RenderTarget>> &renderTargetList)
{
	_renderTargetList.swap(renderTargetList);
}

void RenderPass::initializeTexture(std::vector<std::shared_ptr<TextureComponent>> &textureList)
{
	_textureList.swap(textureList);
}

