#include "Include.h"
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
	: _pOldRenderTargetView{ nullptr }
	, _pOldDepthStencilView{ nullptr }
	, _vertexShader{ std::make_shared<VertexShader>() }
	, _pixelShader{ std::make_shared<PixelShader>() }
	, _geometryShader{ std::make_shared<GeometryShader>() }
	, _bShaderSet{ false }
	, bClearTargets{ true }
	, _bUseOwningDepthStencilBuffer{ false }
{
}

RenderPass::~RenderPass()
{
}

void RenderPass::Begin()
{
	// 기존 정보 저장
	g_pGraphicDevice->getContext()->OMGetRenderTargets(1, &_pOldRenderTargetView, &_pOldDepthStencilView);

	if (bRenderTarget)
	{
		uint32 RenderTargetNum = CastValue<int32>(CachedRenderTargets.size());
		std::vector<ID3D11RenderTargetView*> RowRenderTargets(RenderTargetNum, nullptr);
		//std::vector<ID3D11RenderTargetView*> RowRenderTargets;

		for (const FResourceViewBindData ResourceViewBindData : _renderTargetList)
		{
			RowRenderTargets[ResourceViewBindData.Index] = ResourceViewBindData.ReourceView->AsRenderTargetView();

			if (true == bClearTargets)
			{
				g_pGraphicDevice->getContext()->ClearRenderTargetView(ResourceViewBindData.ReourceView->AsRenderTargetView(), reinterpret_cast<const float*>(&EngineColors::Black));
				g_pGraphicDevice->getContext()->ClearDepthStencilView(ResourceViewBindData.ReourceView->getDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
			}
		}

		g_pGraphicDevice->getContext()->OMSetRenderTargets(RenderTargetNum, RowRenderTargets.data(), _bUseOwningDepthStencilBuffer ? CachedRenderTargets[static_cast<int>(ERenderTarget::ShadowDepth)]->getDepthStencilView() : _pOldDepthStencilView);
	}

	uint32 ResorceViewNum = CastValue<uint32>(CachedResourceViews.size());
	for(const FResourceViewBindData& ResourceViewBindData : _resourceViewList)
	{
		g_pGraphicDevice->getContext()->PSSetShaderResources(ResourceViewBindData.Index, 1, &ResourceViewBindData.ReourceView->AsTexture()->getRawResourceViewPointer());
	}
}

void RenderPass::End()
{
	//uint32 ResorceViewNum = CastValue<uint32>(CachedResourceViews.size());
	std::vector<ID3D11ShaderResourceView*> RowResourceViews(8, nullptr);
	g_pGraphicDevice->getContext()->PSSetShaderResources(0, 8, RowResourceViews.data());

	uint32 renderTargetCount = CachedRenderTargets.size() == 0 ? 1 : CachedRenderTargets.size();
	std::vector<ID3D11RenderTargetView*> restoreRenderTargetViewArray(renderTargetCount, nullptr);
	restoreRenderTargetViewArray[0] = _pOldRenderTargetView;
	g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(renderTargetCount), restoreRenderTargetViewArray.data(), _pOldDepthStencilView);

	//// 쉐이더 리소스 뷰 해제
	//for (const FResourceViewBindData& ResourceViewBindData : _resourceViewList)
	//{
	//	ID3D11ShaderResourceView *pNullShaderResouceView = nullptr;
	//	g_pGraphicDevice->getContext()->PSSetShaderResources(ResourceViewBindData.Index, 1, &pNullShaderResouceView);
	//}

	//// 렌더 타겟 해제와 기존 정보 복구
	//uint32 renderTargetCount = _renderTargetList.size();
	//if (renderTargetCount> 0)
	//{
	//	std::vector<ID3D11RenderTargetView*> restoreRenderTargetViewArray(renderTargetCount, nullptr);
	//	restoreRenderTargetViewArray[0] = _pOldRenderTargetView;
	//	g_pGraphicDevice->getContext()->OMSetRenderTargets(renderTargetCount, restoreRenderTargetViewArray.data(), _pOldDepthStencilView);
	//}

	SafeRelease(_pOldRenderTargetView);
	SafeRelease(_pOldDepthStencilView);
}

void RenderPass::DoPass(const std::vector<FPrimitiveData>& PrimitiveDatList)
{
	//for (auto& primitive : renderQueue)
	//{
	//	// 컴포넌트가 가지고 있는 PrimitiveData 목록을 가져옴
	//	std::vector<FPrimitiveData> primitiveDataList;
	//	primitive->GetPrimitiveData(primitiveDataList);

	for (auto& PrimitiveData : PrimitiveDatList)
	{
		if (processPrimitiveData(PrimitiveData))
		{
			PrimitiveData._pMaterial->getVertexShader()->UpdateConstantBuffer(EConstantBufferLayer::PerObject);
			PrimitiveData._pMaterial->getPixelShader()->UpdateConstantBuffer(EConstantBufferLayer::PerObject);
			render(PrimitiveData);
		}
	}
	//}
}

const bool RenderPass::processPrimitiveData(const FPrimitiveData& primitiveData)
{
	std::shared_ptr<PrimitiveComponent> Primitive = primitiveData._pPrimitive.lock();

	// -------------------------------------------------------------------------------------------------------------------------
	// 버텍스쉐이더 ConstantBuffer
	primitiveData._pMaterial->getVertexShader()->SetValue(TEXT("worldMatrix"), Primitive->getWorldMatrix());
	// 애님 관련 변수
	BOOL animated = primitiveData._matrices != nullptr;
	primitiveData._pMaterial->getVertexShader()->SetValue(TEXT("animated"), animated);
	if (animated == TRUE)
	{
		primitiveData._pMaterial->getVertexShader()->SetValue(TEXT("keyFrameMatrices"), primitiveData._matrices);
	}

	// -------------------------------------------------------------------------------------------------------------------------
	// 픽셀쉐이더 ConstantBuffer
	BOOL bUseNormal = primitiveData._pMaterial->IsTextureTypeUsed(TextureType::Normal) ? TRUE : FALSE;
	primitiveData._pMaterial->getPixelShader()->SetValue(TEXT("bUseNormalTexture"), bUseNormal);
	BOOL bUseSpecular = primitiveData._pMaterial->IsTextureTypeUsed(TextureType::Specular) ? TRUE : FALSE;
	primitiveData._pMaterial->getPixelShader()->SetValue(TEXT("bUseSpecularTexture"), bUseSpecular);
	BOOL bAlphaMask = primitiveData._pMaterial->IsAlphaMasked() ? TRUE : FALSE;
	primitiveData._pMaterial->getPixelShader()->SetValue(TEXT("bAlphaMask"), bAlphaMask);
	return true;
}

void RenderPass::setShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName)
{
	releaseShader();
	std::shared_ptr<VertexShader> vertexShader = nullptr;
	if (ShaderManager->getVertexShader(vertexShaderFileName, vertexShader))
	{
		_vertexShader = vertexShader;
		_vertexShaderFileName = vertexShaderFileName;;
	}
	
	std::shared_ptr<PixelShader> pixelShader = nullptr;
	if (ShaderManager->getPixelShader(pixelShaderFileName, pixelShader))
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
	if (ShaderManager->getGeometryShader(geomtryShaderFileName, geometryShader))
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
	bClearTargets = bClear;
}

void RenderPass::SetUseOwningDepthStencilBuffer(const bool bUse)
{
	_bUseOwningDepthStencilBuffer = bUse;
}

