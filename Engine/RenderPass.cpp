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
	// ���� ���� ����
	g_pGraphicDevice->getContext()->OMGetRenderTargets(1, &_pOldRenderTargetView, &_pOldDepthStencilView);

	// ���� Ÿ�� ����
	uint32 renderTargetCount = sizeToUint32(_renderTargetList.size());
	std::vector<ID3D11RenderTargetView*> rowRenderTargetViewArray(renderTargetCount, nullptr);

	for (uint32 i = 0; i < renderTargetCount; ++i)
	{
		g_pGraphicDevice->getContext()->ClearRenderTargetView(_renderTargetList[i]->getRenderTargetView(), reinterpret_cast<const float *>(&Colors::Black));
		g_pGraphicDevice->getContext()->ClearDepthStencilView(_renderTargetList[i]->getDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
		rowRenderTargetViewArray[i] = _renderTargetList[i]->getRenderTargetView();
	}

	g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(renderTargetCount), &rowRenderTargetViewArray[0], _pOldDepthStencilView);

	// ���̴� ���ҽ� �� ����
	uint32 textureCount = sizeToUint32(_textureList.size());
	for (uint32 i = 0; i < textureCount; ++i)
	{
		g_pGraphicDevice->getContext()->PSSetShaderResources(i, 1, &_textureList[i]->getResourceViewRowPointer());
	}
}

void RenderPass::end()
{
	// ���̴� ���ҽ� �� ����
	uint32 textureCount = sizeToUint32(_textureList.size());
	for (uint32 i = 0; i < textureCount; ++i)
	{
		ID3D11ShaderResourceView *pNullShaderResouceView = nullptr;
		g_pGraphicDevice->getContext()->PSSetShaderResources(i, 1, &pNullShaderResouceView);
	}

	// ���� Ÿ�� ������ ���� ���� ����
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

