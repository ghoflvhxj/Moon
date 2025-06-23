#include "Include.h"
#include "RenderPass.h"

#include "MainGameSetting.h"
#include "Renderer.h"

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

MRenderPass::MRenderPass()
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

MRenderPass::~MRenderPass()
{
}

void MRenderPass::RenderPass(const std::vector<FPrimitiveData>& PrimitiveDatList)
{
    Begin();

    for (auto& PrimitiveData : PrimitiveDatList)
    {
        if (IsValidPrimitive(PrimitiveData) == false)
        {
            continue;
        }

        UpdateTickConstantBuffer(PrimitiveData);
        UpdateObjectConstantBuffer(PrimitiveData);
        DrawPrimitive(PrimitiveData);
    }

    End();
}

void MRenderPass::Begin()
{
	// 기존 정보 저장
	g_pGraphicDevice->getContext()->OMGetRenderTargets(1, &_pOldRenderTargetView, &_pOldDepthStencilView);

	if (bRenderTarget)
	{
		std::vector<ID3D11RenderTargetView*> RawRenderTargets;
        RawRenderTargets.reserve(RenderTargetViewData.size());
		for (const FViewBindData ViewBindData : RenderTargetViewData)
		{
			RawRenderTargets.push_back(ViewBindData.ReourceView->AsRenderTargetView());

			if (true == bClearTargets)
			{
				g_pGraphicDevice->getContext()->ClearRenderTargetView(ViewBindData.ReourceView->AsRenderTargetView(), reinterpret_cast<const float*>(&Color));
				g_pGraphicDevice->getContext()->ClearDepthStencilView(ViewBindData.ReourceView->getDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
			}
		}

		g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(RawRenderTargets.size()), 
            RawRenderTargets.data(), 
            UsedDepthStencilBuffer != ERenderTarget::Count ? CachedRenderTargets[EnumToIndex(UsedDepthStencilBuffer)]->getDepthStencilView() : _pOldDepthStencilView);
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

void MRenderPass::End()
{
    // 쉐이더 리소스 뷰 해제
    uint32 ResorceViewNum = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
	std::vector<ID3D11ShaderResourceView*> RowResourceViews(ResorceViewNum, nullptr);
	g_pGraphicDevice->getContext()->PSSetShaderResources(0, ResorceViewNum, RowResourceViews.data());

    // 렌더 타겟 해제와 기존 정보 복구
	uint32 RenderTargetNum = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
	std::vector<ID3D11RenderTargetView*> restoreRenderTargetViewArray(RenderTargetNum, nullptr);
	restoreRenderTargetViewArray[0] = _pOldRenderTargetView;
	g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(RenderTargetNum), restoreRenderTargetViewArray.data(), _pOldDepthStencilView);

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

bool MRenderPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    const std::shared_ptr<MPrimitiveComponent>& Primitive = PrimitiveData.PrimitiveComponent.lock();
    if (Primitive == nullptr)
    {
        return false;
    }

    std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock();
    if (Material == nullptr)
    {
        return false;
    }

    return true;
}

void MRenderPass::UpdateTickConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock();

    //------------------------------------------------------------------------------------------------------------------
    // 버텍스 쉐이더 CBuffer
    Material->getVertexShader()->SetValue(TEXT("viewMatrix"), g_pMainGame->getMainCameraViewMatrix());
    Material->getVertexShader()->SetValue(TEXT("projectionMatrix"), g_pMainGame->getMainCameraProjectioinMatrix());
    Material->getVertexShader()->SetValue(TEXT("identityMatrix"), IDENTITYMATRIX);
    Material->getVertexShader()->SetValue(TEXT("orthographicProjectionMatrix"), g_pMainGame->getMainCameraOrthographicProjectionMatrix());
    Material->getVertexShader()->SetValue(TEXT("inverseOrthographicProjectionMatrix"), g_pMainGame->getMainCamera()->getInverseOrthographicProjectionMatrix());
}

void MRenderPass::UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData)
{
	const std::shared_ptr<MPrimitiveComponent>& Primitive = PrimitiveData.PrimitiveComponent.lock();
    std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock();

	// -------------------------------------------------------------------------------------------------------------------------
	// 버텍스쉐이더 ConstantBuffer
	Material->getVertexShader()->SetValue(TEXT("worldMatrix"), Primitive->getWorldMatrix());
    Material->getVertexShader()->SetValue(TEXT("inverseWorldMatrix"), Primitive->GetInverseWorldMatrix());
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

void MRenderPass::DrawPrimitive(const FPrimitiveData& PrimitiveData)
{
    HandleInputAssemblerStage(PrimitiveData);
    HandleVertexShaderStage(PrimitiveData);
    HandleGeometryShaderStage(PrimitiveData);
    HandlePixelShaderStage(PrimitiveData);
    HandleRasterizerStage(PrimitiveData);
    HandleOuputMergeStage(PrimitiveData);

    g_pGraphicDevice->getContext()->Draw(PrimitiveData.VertexBuffer->getVertexCount(), 0);
}

void MRenderPass::HandleInputAssemblerStage(const FPrimitiveData& PrimitiveData)
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

    const std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock();
     g_pGraphicDevice->getContext()->IASetPrimitiveTopology(Material->getTopology());
}

void MRenderPass::HandleVertexShaderStage(const FPrimitiveData& PrimitiveData)
{
    std::shared_ptr<MShader>& VertexShader = _vertexShader != nullptr ? _vertexShader : PrimitiveData.Material.lock()->getVertexShader();
    VertexShader->UpdateConstantBuffer(EConstantBufferLayer::Object);
    VertexShader->Apply();
}

void MRenderPass::HandleGeometryShaderStage(const FPrimitiveData& PrimitiveData)
{
    std::shared_ptr<MShader>& GeometryShader = _geometryShader != nullptr ? _geometryShader : nullptr;
    if (GeometryShader == nullptr)
    {
        g_pGraphicDevice->getContext()->GSSetShader(nullptr, nullptr, 0);
    }
    else
    {
        GeometryShader->UpdateConstantBuffer(EConstantBufferLayer::Object);
        GeometryShader->Apply();
    }
}

void MRenderPass::HandlePixelShaderStage(const FPrimitiveData& PrimitiveData)
{
    std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock();

    std::shared_ptr<MShader>& PixelShader = _pixelShader != nullptr ? _pixelShader : Material->getPixelShader();
    PixelShader->UpdateConstantBuffer(EConstantBufferLayer::Object);
    PixelShader->Apply();

    Material->SetTexturesToDevice();

    for (const FViewBindData& Data : ResourceViewData)
    {
        g_pGraphicDevice->getContext()->PSSetShaderResources(Data.Index, 1, &Data.ReourceView->AsTexture()->getRawResourceViewPointer());
    }
}

void MRenderPass::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    const std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock();

    g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Material->getFillMode(), Material->getCullMode()));
}

void MRenderPass::HandleOuputMergeStage(const FPrimitiveData& PrimitiveData)
{
    // DepthStencilState
    g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::EDepthWriteMode::Enable), 1);
    g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Object), nullptr, 0xffffffff);
}

void MRenderPass::setShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName)
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

void MRenderPass::setShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName, const wchar_t *geomtryShaderFileName)
{
	setShader(vertexShaderFileName, pixelShaderFileName);

	std::shared_ptr<MGeometryShader> geometryShader = nullptr;
	if (ShaderManager->getGeometryShader(geomtryShaderFileName, geometryShader))
	{
		_geometryShader = geometryShader;
		_geometryShaderFileName = geomtryShaderFileName;;
	}
}

const bool MRenderPass::isShaderSet() const
{
	return _bShaderSet;
}

void MRenderPass::releaseShader()
{
	_vertexShader = nullptr;
	_pixelShader = nullptr;

	_vertexShaderFileName.clear();
	_pixelShaderFileName.clear();
}

void MRenderPass::SetClearTargets(const bool bClear)
{
	bClearTargets = bClear;
}

void MRenderPass::SetUseOwningDepthStencilBuffer(const ERenderTarget bUse)
{
	UsedDepthStencilBuffer = bUse;
}

