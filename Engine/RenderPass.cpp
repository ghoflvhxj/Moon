#include "stdafx.h"
#include "RenderPass.h"

// Graphic
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"
#include "VertexShader.h"
#include "PixelShader.h"

// Render
#include "RenderTarget.h"

// Game
#include "MainGame.h"

// Actor
#include "Camera.h"

#include "TextureComponent.h"

RenderPass::RenderPass()
	: _renderTargetList(RT_COUNT, nullptr)
	, _resourceViewList(RT_COUNT, nullptr)
	, _pOldRenderTargetView{ nullptr }
	, _pOldDepthStencilView{ nullptr }
	, _vertexShader{ std::make_shared<VertexShader>() }
	, _pixelShader{ std::make_shared<PixelShader>() }
	, _bShaderSet{ false }
	, _bRenderTargetSet{ false }
	, _renderTargetCount{ RT_COUNT }
	, _bClearTargets{ true }
{
}

RenderPass::~RenderPass()
{
}

void RenderPass::begin()
{
	// ±âÁ¸ Á¤º¸ ÀúÀå
	g_pGraphicDevice->getContext()->OMGetRenderTargets(1, &_pOldRenderTargetView, &_pOldDepthStencilView);

	// ·»´õ Å¸°Ù ¼³Á¤
	if (true == _bRenderTargetSet)
	{
		std::vector<ID3D11RenderTargetView*> rowRenderTargetViewArray(_renderTargetCount, nullptr);

		for (uint32 i = 0; i < _renderTargetCount; ++i)
		{
			if (_renderTargetList[i] == nullptr)
			{
				continue;
			}

			if (true == _bClearTargets)
			{
				g_pGraphicDevice->getContext()->ClearRenderTargetView(_renderTargetList[i]->AsRenderTargetView(), reinterpret_cast<const float *>(&EngineColors::Black));
				g_pGraphicDevice->getContext()->ClearDepthStencilView(_renderTargetList[i]->getDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
			}
			rowRenderTargetViewArray[i] = _renderTargetList[i]->AsRenderTargetView();
		}

		g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(_renderTargetCount), &rowRenderTargetViewArray[0], _pOldDepthStencilView);
	}

	// ½¦ÀÌ´õ ¸®¼Ò½º ºä ¼³Á¤
	uint32 resourceViewCount = CastValue<uint32>(_resourceViewList.size());
	for (uint32 i = 0; i < resourceViewCount; ++i)
	{
		if (_resourceViewList[i] == nullptr)
		{
			continue;
		}

		g_pGraphicDevice->getContext()->PSSetShaderResources(i, 1, &_resourceViewList[i]->AsTexture()->getRawResourceViewPointer());
	}
}

void RenderPass::end()
{
	// ½¦ÀÌ´õ ¸®¼Ò½º ºä ÇØÁ¦
	uint32 resorceViewCount = CastValue<uint32>(_resourceViewList.size());
	for (uint32 i = 0; i < resorceViewCount; ++i)
	{
		ID3D11ShaderResourceView *pNullShaderResouceView = nullptr;
		g_pGraphicDevice->getContext()->PSSetShaderResources(i, 1, &pNullShaderResouceView);
	}

	// ·»´õ Å¸°Ù ÇØÁ¦¿Í ±âÁ¸ Á¤º¸ º¹±¸
	uint32 renderTargetCount = (_renderTargetCount == 0) ? 1 : _renderTargetCount;
	std::vector<ID3D11RenderTargetView*> restoreRenderTargetViewArray(renderTargetCount, nullptr);
	restoreRenderTargetViewArray[0] = _pOldRenderTargetView;
	g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(renderTargetCount), &restoreRenderTargetViewArray[0], _pOldDepthStencilView);

	SafeRelease(_pOldRenderTargetView);
	SafeRelease(_pOldDepthStencilView);
}

void RenderPass::doPass(RenderQueue &renderQueue)
{
	for (auto& primitive : renderQueue)
	{
		// ÄÄÆ÷³ÍÆ®°¡ °¡Áö°í ÀÖ´Â PrimitiveData ¸ñ·ÏÀ» °¡Á®¿È
		std::vector<PrimitiveData> primitiveDataList = {};
		primitive->getPrimitiveData(primitiveDataList);

		for (auto &primitiveData : primitiveDataList)
		{
			if (processPrimitiveData(primitiveData))
			{
				render(primitiveData);
			}
		}
	}
}

void RenderPass::setShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName)
{
	releaseShader();
	std::shared_ptr<VertexShader> vertexShader = nullptr;
	if (g_pShaderManager->getVertexShader(vertexShaderFileName, vertexShader))
	{
		_vertexShader = vertexShader;
		_vertexShaderFileName = vertexShaderFileName;;
	}
	
	std::shared_ptr<PixelShader> pixelShader = nullptr;
	if (g_pShaderManager->getPixelShader(pixelShaderFileName, pixelShader))
	{
		_pixelShader = pixelShader;
		_pixelShaderFileName = pixelShaderFileName;
	}
	else
	{
		_pixelShader = std::make_shared<PixelShader>();
	}

	_bShaderSet = true;
}

const bool RenderPass::isShaderSet() const
{
	return _bShaderSet;
}

void RenderPass::releaseShader()
{
	_vertexShader = nullptr;
	_pixelShader = nullptr;

	_vertexShaderFileName.clear();
	_pixelShaderFileName.clear();
}

void RenderPass::SetClearTargets(const bool bClear)
{
	_bClearTargets = bClear;
}

