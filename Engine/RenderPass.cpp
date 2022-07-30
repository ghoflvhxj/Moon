#include "stdafx.h"
#include "RenderPass.h"

// Graphic
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"

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
	, _vertexShader{ nullptr }
	, _pixelShader{ nullptr }
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
				g_pGraphicDevice->getContext()->ClearRenderTargetView(_renderTargetList[i]->AsRenderTargetView(), reinterpret_cast<const float *>(&Colors::Black));
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

void RenderPass::render(PrimitiveData &primitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != primitiveData._pVertexBuffer)
	{
		primitiveData._pVertexBuffer->setBufferToDevice(stride, offset);
	}

	if (nullptr != primitiveData._pIndexBuffer)
	{
		primitiveData._pIndexBuffer->setBufferToDevice(offset);
	}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(primitiveData._pMaterial->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variableInfosVS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	primitiveData._pVertexShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosVS);
	primitiveData._pVertexShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
	primitiveData._pPixelShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosPS);
	primitiveData._pPixelShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// RasterizerState
	g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));

	//--------------------------------------------------------------------------------------------------------------------------------
	// DepthStencilState
	g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::DepthWriteMode::Enable), 1);

	//--------------------------------------------------------------------------------------------------------------------------------
	// OutputMerge
	g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Object), nullptr, 0xffffffff);

	//--------------------------------------------------------------------------------------------------------------------------------
	//g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(primitiveData._pIndexBuffer->getIndexCount())
	//	, static_cast<UINT>(_indexOffsetList[indexOffsetCount - 1])
	//	, static_cast<UINT>(_vertexOffsetList[indexOffsetCount - 1]));

	g_pGraphicDevice->getContext()->Draw(primitiveData._pVertexBuffer->getVertexCount(), 0);
}

void RenderPass::setShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName)
{
	releaseShader();

	if (0 == wcscmp(vertexShaderFileName, TEXT("")))
	{
		_vertexShader = nullptr;
	}
	else
	{
		if (false == g_pShaderManager->getVertexShader(vertexShaderFileName, _vertexShader))
		{
			return;
		}
	}
	_vertexShaderFileName = vertexShaderFileName;

	if (0 == wcscmp(pixelShaderFileName, TEXT("")))
	{
		_pixelShader = nullptr;
	}
	else
	{
		if (false == g_pShaderManager->getPixelShader(pixelShaderFileName, _pixelShader))
		{
			return;
		}
	}
	_pixelShaderFileName = pixelShaderFileName;

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

