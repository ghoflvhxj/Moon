#include "stdafx.h"
#include "RenderPass.h"

// Graphic
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "GeometryShader.h"

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
	, _geometryShader{ std::make_shared<GeometryShader>() }
	, _bShaderSet{ false }
	, _bRenderTargetSet{ false }
	, _renderTargetCount{ RT_COUNT }
	, _bClearTargets{ true }
	, _bUseOwningDepthStencilBuffer{ false }
{
}

RenderPass::~RenderPass()
{
}

void RenderPass::begin()
{
	// 기존 정보 저장
	g_pGraphicDevice->getContext()->OMGetRenderTargets(1, &_pOldRenderTargetView, &_pOldDepthStencilView);

	// 렌더 타겟 설정
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

		if (_bUseOwningDepthStencilBuffer)
		{
			// 6 하드 코딩했으니 수정 필요
			g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(_renderTargetCount), &rowRenderTargetViewArray[0], _renderTargetList[6]->getDepthStencilView());
		}
		else
		{
			g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(_renderTargetCount), &rowRenderTargetViewArray[0], _pOldDepthStencilView);
		}
	}

	// 쉐이더 리소스 뷰 설정
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
	// 쉐이더 리소스 뷰 해제
	uint32 resorceViewCount = CastValue<uint32>(_resourceViewList.size());
	for (uint32 i = 0; i < resorceViewCount; ++i)
	{
		ID3D11ShaderResourceView *pNullShaderResouceView = nullptr;
		g_pGraphicDevice->getContext()->PSSetShaderResources(i, 1, &pNullShaderResouceView);
	}

	// 렌더 타겟 해제와 기존 정보 복구
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
		// 컴포넌트가 가지고 있는 PrimitiveData 목록을 가져옴
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

void RenderPass::setShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName, const wchar_t *geomtryShaderFileName)
{
	setShader(vertexShaderFileName, pixelShaderFileName);

	std::shared_ptr<GeometryShader> geometryShader = nullptr;
	if (g_pShaderManager->getGeometryShader(geomtryShaderFileName, geometryShader))
	{
		_geometryShader = geometryShader;
		_geometryShaderFileName = geomtryShaderFileName;;
	}
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

void RenderPass::SetUseOwningDepthStencilBuffer(const bool bUse)
{
	_bUseOwningDepthStencilBuffer = bUse;
}

