#include "Include.h"
#include "RenderPass.h"

#include "MainGameSetting.h"

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

#include "Texture.h"

RenderPass::RenderPass()
	: _pOldRenderTargetView{ nullptr }
	, _pOldDepthStencilView{ nullptr }
	, _vertexShader{ nullptr }
	, _pixelShader{ nullptr }
	, _geometryShader{ nullptr }
	, _bShaderSet{ false }
	, bClearTargets{ true }
	, UsedDepthStencilBuffer{ ERenderTarget::Count }
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
		std::vector<ID3D11RenderTargetView*> RowRenderTargets;
		for (const FViewBindData ViewBindData : RenderTargetViewData)
		{
			RowRenderTargets.push_back(ViewBindData.ReourceView->AsRenderTargetView());

			if (true == bClearTargets)
			{
				g_pGraphicDevice->getContext()->ClearRenderTargetView(ViewBindData.ReourceView->AsRenderTargetView(), reinterpret_cast<const float*>(&Color));
				g_pGraphicDevice->getContext()->ClearDepthStencilView(ViewBindData.ReourceView->getDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
			}
		}
		
		// PointRenderPass에서, CachedRenderTargets[static_cast<int>(ERenderTarget::DirectionalShadowDepth)]를 고정적으로 사용하고 있음...
		g_pGraphicDevice->getContext()->OMSetRenderTargets(RowRenderTargets.size(), RowRenderTargets.data(), UsedDepthStencilBuffer != ERenderTarget::Count ? CachedRenderTargets[enumToIndex(UsedDepthStencilBuffer)]->getDepthStencilView() : _pOldDepthStencilView);
	}


	if (RenderTargetViewData.size() > 0)
	{
		uint32 Width = 0, Height = 0;
		RenderTargetViewData[0].ReourceView->AsTexture()->GetResolution(Width, Height);
		D3D11_VIEWPORT Viewport;
		UINT ViewportNum = 0;
		Viewport.Width = static_cast<float>(Width);
		Viewport.Height = static_cast<float>(Height);
		Viewport.TopLeftX = 0.f;
		Viewport.TopLeftY = 0.f;
		Viewport.MinDepth = 0.f;
		Viewport.MaxDepth = 1.f;
		g_pGraphicDevice->getContext()->RSSetViewports(1, &Viewport);
	}
}

void RenderPass::End()
{
    uint32 ResorceViewNum = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
	std::vector<ID3D11ShaderResourceView*> RowResourceViews(ResorceViewNum, nullptr);
	g_pGraphicDevice->getContext()->PSSetShaderResources(0, ResorceViewNum, RowResourceViews.data());

	uint32 RenderTargetNum = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
	std::vector<ID3D11RenderTargetView*> restoreRenderTargetViewArray(RenderTargetNum, nullptr);
	restoreRenderTargetViewArray[0] = _pOldRenderTargetView;
	g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(RenderTargetNum), restoreRenderTargetViewArray.data(), _pOldDepthStencilView);

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

	D3D11_VIEWPORT Viewport;
	Viewport.Width = g_pSetting->getResolutionWidth<FLOAT>();
	Viewport.Height = g_pSetting->getResolutionHeight<FLOAT>();
	Viewport.TopLeftX = 0.f;
	Viewport.TopLeftY = 0.f;
	Viewport.MinDepth = 0.f;
	Viewport.MaxDepth = 1.f;
	g_pGraphicDevice->getContext()->RSSetViewports(1, &Viewport);

	SafeRelease(_pOldRenderTargetView);
	SafeRelease(_pOldDepthStencilView);
}

void RenderPass::Render(const std::vector<FPrimitiveData>& PrimitiveDatList)
{
	for (auto& PrimitiveData : PrimitiveDatList)
	{
		if (IsValidPrimitive(PrimitiveData))
		{
            UpdateObjectConstantBuffer(PrimitiveData);
			DrawPrimitive(PrimitiveData);
		}
	}
}

bool RenderPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    const std::shared_ptr<PrimitiveComponent>& Primitive = PrimitiveData._pPrimitive.lock();
    if (Primitive == nullptr)
    {
        return false;
    }

    std::shared_ptr<MMaterial>& Material = PrimitiveData._pMaterial.lock();
    if (Material == nullptr)
    {
        return false;
    }

    return true;
}

void RenderPass::UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData)
{
	const std::shared_ptr<PrimitiveComponent>& Primitive = PrimitiveData._pPrimitive.lock();
    std::shared_ptr<MMaterial>& Material = PrimitiveData._pMaterial.lock();

	// -------------------------------------------------------------------------------------------------------------------------
	// 버텍스쉐이더 ConstantBuffer
	Material->getVertexShader()->SetValue(TEXT("worldMatrix"), Primitive->getWorldMatrix());
	// 애님 관련 변수
	BOOL animated = PrimitiveData._matrices != nullptr;
	Material->getVertexShader()->SetValue(TEXT("animated"), animated);
	if (animated == TRUE)
	{
		Material->getVertexShader()->SetValue(TEXT("keyFrameMatrices"), PrimitiveData._matrices);
	}

	// -------------------------------------------------------------------------------------------------------------------------
	// 픽셀쉐이더 ConstantBuffer
	BOOL bUseNormal = Material->IsTextureTypeUsed(ETextureType::Normal) ? TRUE : FALSE;
	Material->getPixelShader()->SetValue(TEXT("bUseNormalTexture"), bUseNormal);
	BOOL bUseSpecular = Material->IsTextureTypeUsed(ETextureType::Specular) ? TRUE : FALSE;
	Material->getPixelShader()->SetValue(TEXT("bUseSpecularTexture"), bUseSpecular);
	BOOL bAlphaMask = Material->IsAlphaMasked() ? TRUE : FALSE;
	Material->getPixelShader()->SetValue(TEXT("bAlphaMask"), bAlphaMask);
}

void RenderPass::DrawPrimitive(const FPrimitiveData& PrimitiveData)
{
    HandleInputAssemblerStage(PrimitiveData);
    HandleVertexShaderStage(PrimitiveData);
    HandleGeometryShaderStage(PrimitiveData);
    HandlePixelShaderStage(PrimitiveData);
    HandleRasterizerStage(PrimitiveData);
    HandleOuputMergeStage(PrimitiveData);

    g_pGraphicDevice->getContext()->Draw(PrimitiveData.VertexBuffer->getVertexCount(), 0);
}

void RenderPass::HandleInputAssemblerStage(const FPrimitiveData& PrimitiveData)
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    // IA에 버텍스 버퍼 설정
    PrimitiveData.VertexBuffer->setBufferToDevice(stride, offset);

    // IA에 인덱스 버퍼 설정
    if (nullptr != PrimitiveData.IndexBuffer)
    {
        //PrimitiveData._pIndexBuffer->setBufferToDevice();
    }

    const std::shared_ptr<MMaterial>& Material = PrimitiveData._pMaterial.lock();
     g_pGraphicDevice->getContext()->IASetPrimitiveTopology(Material->getTopology());
}

void RenderPass::HandleVertexShaderStage(const FPrimitiveData& PrimitiveData)
{
    std::shared_ptr<MShader>& VertexShader = _vertexShader != nullptr ? _vertexShader : PrimitiveData._pMaterial.lock()->getVertexShader();
    VertexShader->UpdateConstantBuffer(EConstantBufferLayer::PerObject);
    VertexShader->SetToDevice();
}

void RenderPass::HandleGeometryShaderStage(const FPrimitiveData& PrimitiveData)
{
    std::shared_ptr<MShader>& GeometryShader = _geometryShader != nullptr ? _geometryShader : nullptr;
    if (GeometryShader == nullptr)
    {
        g_pGraphicDevice->getContext()->GSSetShader(nullptr, nullptr, 0);
    }
    else
    {
        GeometryShader->UpdateConstantBuffer(EConstantBufferLayer::PerObject);
        GeometryShader->SetToDevice();
    }
}

void RenderPass::HandlePixelShaderStage(const FPrimitiveData& PrimitiveData)
{
    std::shared_ptr<MMaterial>& Material = PrimitiveData._pMaterial.lock();

    std::shared_ptr<MShader>& PixelShader = _pixelShader != nullptr ? _pixelShader : Material->getPixelShader();
    PixelShader->UpdateConstantBuffer(EConstantBufferLayer::PerObject);
    PixelShader->SetToDevice();

    Material->SetTexturesToDevice();

    for (const FViewBindData& Data : ResourceViewData)
    {
        g_pGraphicDevice->getContext()->PSSetShaderResources(Data.Index, 1, &Data.ReourceView->AsTexture()->getRawResourceViewPointer());
    }
}

void RenderPass::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    const std::shared_ptr<MMaterial>& Material = PrimitiveData._pMaterial.lock();

    g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Material->getFillMode(), Material->getCullMode()));
}

void RenderPass::HandleOuputMergeStage(const FPrimitiveData& PrimitiveData)
{
    // DepthStencilState
    g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::EDepthWriteMode::Enable), 1);
    g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Object), nullptr, 0xffffffff);
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

	std::shared_ptr<MGeometryShader> geometryShader = nullptr;
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

void RenderPass::SetUseOwningDepthStencilBuffer(const ERenderTarget bUse)
{
	UsedDepthStencilBuffer = bUse;
}

