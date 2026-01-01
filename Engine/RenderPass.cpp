#include "RenderPass.h"

#include "MoonEngine.h"

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
#include "World.h"

// Actor
#include "Camera.h"

#include "Texture.h"

#include "DynamicMeshComponent.h"

using namespace DirectX;

MRenderPass::MRenderPass()
	: _vertexShader{ nullptr }
	, _pixelShader{ nullptr }
	, _geometryShader{ nullptr }
	, _bShaderSet{ false }
	, bClearTargets{ true }
	, UseOwningDepthStencilBuffer{ ERenderTarget::Count }
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

        UpdateObjectConstantBuffer(PrimitiveData);
        DrawPrimitive(PrimitiveData);
    }

    End();
}

void MRenderPass::Begin()
{
	if (bRenderTarget)
	{
		std::vector<ID3D11RenderTargetView*> RawRenderTargets;
        RawRenderTargets.reserve(RenderTargetViewData.size());

		for (const FRenderTargetBindData& BindData : RenderTargetViewData)
		{
            auto& RenderTarget = getRenderer()->GetRenderTarget(BindData.Index);

			RawRenderTargets.push_back(RenderTarget->AsRenderTargetView());

			if (true == bClearTargets)
			{
                g_pGraphicDevice->ClearRenderTarget(RenderTarget, Color);
			}
		}

        g_pGraphicDevice->getContext()->OMSetRenderTargets(
            static_cast<UINT>(RawRenderTargets.size()),
            RawRenderTargets.data(),
            //UseOwningDepthStencilBuffer != ERenderTarget::Count ? getRenderer()->GetRenderTarget(UseOwningDepthStencilBuffer)->getDepthStencilView() : g_pGraphicDevice->GetDepthStencilView()
            UseOwningDepthStencilBuffer != ERenderTarget::Count ? getRenderer()->GetRenderTarget(UseOwningDepthStencilBuffer)->getDepthStencilView() : (bUseCommonDepthStencil ? g_pGraphicDevice->GetDepthStencilView() : nullptr)
        );
	}

    auto& ViewportSize = getGraphicDevice()->GetViewportSize();
    RectWidth = std::get<0>(ViewportSize);
    RectHeight = std::get<1>(ViewportSize);

	if (RenderTargetViewData.size() > 0)
	{
        uint32 Width = 0, Height = 0;
        getRenderer()->GetRenderTarget(RenderTargetViewData[0].Index)->AsTexture()->GetResolution(Width, Height);

        D3D11_VIEWPORT Viewport = {};
		UINT ViewportNum = 0;
		Viewport.Width = static_cast<FLOAT>(Width);
		Viewport.Height = static_cast<FLOAT>(Height);
		Viewport.TopLeftX = 0.f;
		Viewport.TopLeftY = 0.f;
		Viewport.MinDepth = 0.f;
		Viewport.MaxDepth = 1.f;
		g_pGraphicDevice->getContext()->RSSetViewports(1, &Viewport);

        RectWidth = Width;
        RectHeight = Height;
	}

    UINT RectNum = 1;
    D3D11_RECT Rect = {};
    Rect.left = 0;
    Rect.top = 0;
    Rect.right = RectWidth;
    Rect.bottom = RectHeight;
    getGraphicDevice()->getContext()->RSSetScissorRects(RectNum, &Rect);
}

void MRenderPass::End()
{
    g_pGraphicDevice->SetToDefault();
}

bool MRenderPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    const std::shared_ptr<MPrimitiveComponent>& Primitive = PrimitiveData.PrimitiveComponent.lock();
    if (Primitive != nullptr)
    {
        if (Primitive->IsRendering() == false)
        {
            return false;
        }

        std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock();
        if (Material == nullptr)
        {
            return false;
        }
    }

    //if (PrimitiveData.VertexBuffer.lock() == nullptr)
    //{
    //    return false;
    //}

    return true;
}

void MRenderPass::UpdateTickConstantBuffer(const FPrimitiveData& PrimitiveData)
{
}

void MRenderPass::UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    const auto& Camera = getRenderer()->GetWorld()->getMainCamera();
	const std::shared_ptr<MPrimitiveComponent>& Primitive = PrimitiveData.PrimitiveComponent.lock();

    std::shared_ptr<MShader>& VS = GetVertexShader(PrimitiveData);

    auto GetViewProjMatrix = [](bool bOrtho)->Mat4 {
        return bOrtho ? getRenderer()->ViewOrthogonalProjMatrix : getRenderer()->ViewPerspectiveProjMatrix;
    };

	// -------------------------------------------------------------------------------------------------------------------------
	// 버텍스쉐이더 ConstantBuffer
    if (VS->HasConstantBuffer())
    {
        BOOL animated = FALSE;
        if (Primitive)
        {
            VS->SetValue(TEXT("worldMatrix"), Primitive->getWorldMatrix());

            Mat4 WorldView = {};
            XMStoreFloat4x4(&WorldView, XMLoadFloat4x4(&Primitive->getWorldMatrix()) * XMLoadFloat4x4(&Camera->getViewMatrix()));
            VS->SetValue(TEXT("WorldView"), WorldView);

            Mat4 WorldViewProj = {};
            XMStoreFloat4x4(&WorldViewProj, XMLoadFloat4x4(&Primitive->getWorldMatrix()) * XMLoadFloat4x4(&GetViewProjMatrix(Primitive->getRenderMdoe() == MPrimitiveComponent::ERenderMode::Orthogonal)));
            //const Mat4& Proj = Primitive->getRenderMdoe() == MPrimitiveComponent::ERenderMode::Orthogonal ? Camera->getOrthographicProjectionMatrix() : Camera->getPerspectiveProjectionMatrix();
            //XMStoreFloat4x4(&WorldViewProj, XMLoadFloat4x4(&WorldView) * XMLoadFloat4x4(&Proj));
            VS->SetValue(TEXT("WorldViewProj"), WorldViewProj);

            VS->SetValue(TEXT("inverseWorldMatrix"), Primitive->GetInverseWorldMatrix());
            VS->SetValue(TEXT("bOrtho"), Primitive->getRenderMdoe() == MPrimitiveComponent::ERenderMode::Orthogonal ? TRUE : FALSE);
            if (std::shared_ptr<DynamicMeshComponent> DynamicMeshComp = Primitive->CastToShared<DynamicMeshComponent>())
            {
                animated = DynamicMeshComp->HasAnim() && DynamicMeshComp->bBindPose == false ? TRUE : FALSE;
                if (animated)
                {
                    VS->SetValue(TEXT("keyFrameMatrices"), DynamicMeshComp->GetAnimMatrices());
                }
            }
        }
        else
        {
            Mat4 WorldMatrix = {};
            XMMATRIX XMWorldMat = XMMatrixScalingFromVector(XMLoadFloat3(&PrimitiveData.Scale)) * XMMatrixRotationQuaternion(XMLoadFloat4(&PrimitiveData.Rotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&PrimitiveData.Translation));
            XMStoreFloat4x4(&WorldMatrix, XMWorldMat);
            VS->SetValue(TEXT("worldMatrix"), WorldMatrix);

            Mat4 WorldView = {};
            XMStoreFloat4x4(&WorldView, XMWorldMat * XMLoadFloat4x4(&Camera->getViewMatrix()));
            VS->SetValue(TEXT("WorldView"), WorldView);

            Mat4 WorldViewProj = {};
            XMStoreFloat4x4(&WorldViewProj, XMWorldMat * XMLoadFloat4x4(&GetViewProjMatrix(PrimitiveData.ProjectionType == EProjectionType::Orthograhpic)));
            VS->SetValue(TEXT("WorldViewProj"), WorldViewProj);

            Mat4 InvWorldMatrix = {};
            XMStoreFloat4x4(&InvWorldMatrix, XMMatrixInverse(nullptr, XMWorldMat));
            VS->SetValue(TEXT("inverseWorldMatrix"), InvWorldMatrix);
            VS->SetValue(TEXT("bOrtho"), FALSE);
        }

        VS->SetValue(TEXT("animated"), animated);
        VS->SetValue(TEXT("bInstance"), PrimitiveData.InstanceBuffer.expired() == false ? TRUE : FALSE);
    }

	// -------------------------------------------------------------------------------------------------------------------------
	// 픽셀쉐이더 ConstantBuffer
    std::shared_ptr<MShader>& PS = GetPixelShader(PrimitiveData);
    if (PS->HasConstantBuffer())
    {
        if (std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock())
        {
            BOOL bUseNormal = Material->IsTextureTypeUsed(ETextureType::Normal) ? TRUE : FALSE;
            PS->SetValue(TEXT("bUseNormalTexture"), bUseNormal);
            BOOL bUseSpecular = Material->IsTextureTypeUsed(ETextureType::Specular) ? TRUE : FALSE;
            PS->SetValue(TEXT("bUseSpecularTexture"), bUseSpecular);
            BOOL bAlphaMask = Material->IsAlphaMasked() ? TRUE : FALSE;
            PS->SetValue(TEXT("bAlphaMask"), bAlphaMask);
            BOOL bRimLight = Material->IsRimLighted() ? TRUE : FALSE;
            PS->SetValue(TEXT("bRimLight"), bRimLight);
        }
        else
        {
            PS->SetValue(TEXT("bUseNormalTexture"), FALSE);
            PS->SetValue(TEXT("bUseSpecularTexture"), FALSE);
            PS->SetValue(TEXT("bAlphaMask"), FALSE);
            PS->SetValue(TEXT("bRimLight"), FALSE);
        }
    }
}

void MRenderPass::DrawPrimitive(const FPrimitiveData& PrimitiveData)
{
    HandleInputAssemblerStage(PrimitiveData);
    HandleVertexShaderStage(PrimitiveData);
    HandleGeometryShaderStage(PrimitiveData);
    HandlePixelShaderStage(PrimitiveData);
    HandleRasterizerStage(PrimitiveData);
    HandleOutputMergeStage(PrimitiveData);

    std::shared_ptr<MVertexBuffer>& VertexBuffer = PrimitiveData.VertexBuffer.lock();

    if (std::shared_ptr<MVertexBuffer> InstanceBuffer = PrimitiveData.InstanceBuffer.lock())
    {
        UINT InstanceNum = static_cast<UINT>(InstanceBuffer->getVertexNum());

        if (std::shared_ptr<MIndexBuffer> IndexBuffer = PrimitiveData.IndexBuffer.lock())
        {
            UINT IndexNum = static_cast<UINT>(IndexBuffer->getIndexCount());
            g_pGraphicDevice->getContext()->DrawIndexedInstanced(IndexNum, InstanceNum, 0, 0, 0);
        }
        else
        {
            UINT VertexNum = static_cast<UINT>(VertexBuffer->getVertexNum());
            g_pGraphicDevice->getContext()->DrawInstanced(VertexNum, InstanceNum, 0, 0);
        }
    }
    else
    {
        if (std::shared_ptr<MIndexBuffer> IndexBuffer = PrimitiveData.IndexBuffer.lock())
        {
            g_pGraphicDevice->getContext()->DrawIndexed(IndexBuffer->getIndexCount(), 0, 0);
        }
        else
        {
            g_pGraphicDevice->getContext()->Draw(VertexBuffer->getVertexNum(), 0);
        }
    }

}

void MRenderPass::HandleInputAssemblerStage(const FPrimitiveData& PrimitiveData)
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    std::vector<ID3D11Buffer*> VertexBuffers;
    std::vector<UINT> Strides;
    std::vector<UINT> Offsets;

    // 버텍스 버퍼
    std::shared_ptr<MVertexBuffer>& VertexBuffer = PrimitiveData.VertexBuffer.lock();
    VertexBuffers.push_back(VertexBuffer->getBuffer());
    Strides.push_back(VertexBuffer->GetVertexSize());
    Offsets.push_back(0);

    // 인스턴싱 버퍼
    if (std::shared_ptr<MVertexBuffer>& InstanceBuffer = PrimitiveData.InstanceBuffer.lock())
    {
        VertexBuffers.push_back(InstanceBuffer->getBuffer());
        Strides.push_back(InstanceBuffer->GetVertexSize());
        Offsets.push_back(0);
    }
    else
    {
        //VertexBuffers.push_back(nullptr);
        //Strides.push_back(0);
        //Offsets.push_back(0);
    }

    UINT VertexBufferNum = GetSize(VertexBuffers);
    getGraphicDevice()->getContext()->IASetVertexBuffers(0, VertexBufferNum, VertexBuffers.data(), Strides.data(), Offsets.data());

    //g_pGraphicDevice->getContext()->IASetInputLayout(g_pGraphicDevice->m_pInputLayout);

    // IA에 인덱스 버퍼 설정
    if (std::shared_ptr<MIndexBuffer>& IndexBuffer = PrimitiveData.IndexBuffer.lock())
    {
        IndexBuffer->setBufferToDevice(0);
    }

    if (const std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock())
    {
        g_pGraphicDevice->getContext()->IASetPrimitiveTopology(Material->getTopology());
    }
    else
    {
        g_pGraphicDevice->getContext()->IASetPrimitiveTopology(DefaultTopology);
    }
}

void MRenderPass::HandleVertexShaderStage(const FPrimitiveData& PrimitiveData)
{
    std::shared_ptr<MShader>& VertexShader = GetVertexShader(PrimitiveData);
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
    std::shared_ptr<MShader> PixelShader = GetPixelShader(PrimitiveData);
    PixelShader->UpdateConstantBuffer(EConstantBufferLayer::Object);
    PixelShader->Apply();

    if (std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock())
    {
        Material->SetTexturesToDevice();
    }

    for (const FRenderTargetBindData& Data : ResourceViewData)
    {
        //g_pGraphicDevice->getContext()->PSSetShaderResources(static_cast<UINT>(Data.Index), 1, &getRenderer()->GetRenderTarget(Data.Index)->AsTexture()->getRawResourceViewPointer());
        ID3D11ShaderResourceView* SRV = getRenderer()->GetResourceView(Data.Index);
        g_pGraphicDevice->getContext()->PSSetShaderResources(static_cast<UINT>(Data.Index), 1, &SRV);
    }
}

void MRenderPass::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    if (const std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock())
    {
        g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Material->getFillMode(), Material->getCullMode()));
    }
    else
    {
        g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));
    }
}

void MRenderPass::HandleOutputMergeStage(const FPrimitiveData& PrimitiveData)
{
    uint32 DepthStencilFlag = 0;
    if (bDepthEnable)
    {
        DepthStencilFlag |= (uint32)Graphic::EDepthStencilMode::DepthEnable;
    }
    else
    {
        DepthStencilFlag |= (uint32)Graphic::EDepthStencilMode::DepthDisable;
    }

    UINT StencilRef = 0;
    if (std::shared_ptr<MPrimitiveComponent> PrimitiveComp = PrimitiveData.PrimitiveComponent.lock())
    {
        if (PrimitiveData.PrimitiveComponent.lock()->IsStencil())
        {
            DepthStencilFlag |= (uint32)Graphic::EDepthStencilMode::StencilEnable;
            StencilRef = 1;
        }
        else
        {
            DepthStencilFlag |= (int32)Graphic::EDepthStencilMode::StencilDisable;
        }
    }
    else
    {
        DepthStencilFlag |= (uint32)Graphic::EDepthStencilMode::StencilDisable;
    }

    g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(DepthStencilFlag), StencilRef);
    g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Object), nullptr, 0xffffffff);
}

std::shared_ptr<MShader> MRenderPass::GetVertexShader(const FPrimitiveData& InPrimitiveData)
{
    std::shared_ptr<MMaterial>& Material = InPrimitiveData.Material.lock();
    std::shared_ptr<MShader> OutShader = nullptr;
    if (Material != nullptr && bUseDefaultShaderOnly == false)
    {
        OutShader = Material->getVertexShader();
    }
    else
    {
        OutShader = _vertexShader;
    }

    return OutShader;
}

std::shared_ptr<MShader> MRenderPass::GetPixelShader(const FPrimitiveData& InPrimitiveData)
{
    std::shared_ptr<MMaterial>& Material = InPrimitiveData.Material.lock();
    std::shared_ptr<MShader> OutShader = nullptr;
    if (Material != nullptr && bUseDefaultShaderOnly == false)
    {
        OutShader = Material->getPixelShader();
    }
    else
    {
        OutShader = _pixelShader;
    }

    return OutShader;
}

void MRenderPass::SetDefaultShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName)
{
	releaseShader();
	std::shared_ptr<VertexShader> vertexShader = nullptr;
	if (g_pGraphicDevice->GetVertexShader(vertexShaderFileName, vertexShader))
	{
		_vertexShader = vertexShader;
		_vertexShaderFileName = vertexShaderFileName;;
	}
	
	std::shared_ptr<PixelShader> pixelShader = nullptr;
	if (pixelShaderFileName != nullptr && g_pGraphicDevice->GetPixelShader(pixelShaderFileName, pixelShader))
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

void MRenderPass::SetDefaultShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName, const wchar_t *geomtryShaderFileName)
{
	SetDefaultShader(vertexShaderFileName, pixelShaderFileName);

	std::shared_ptr<MGeometryShader> geometryShader = nullptr;
	if (g_pGraphicDevice->GetGeometryShader(geomtryShaderFileName, geometryShader))
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
	UseOwningDepthStencilBuffer = bUse;
}

