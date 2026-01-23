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
	//, UseOwningDepthStencilBuffer{ ERenderTarget::Count }
{
}

MRenderPass::~MRenderPass()
{
}

void MRenderPass::RenderPass(const std::vector<FPrimitiveData>& PrimitiveDatList)
{ 
    for (auto& PrimitiveData : PrimitiveDatList)
    {
        if (IsValidPrimitive(PrimitiveData) == false)
        {
            continue;
        }

        UpdateRenderPassConstantBuffer(PrimitiveData);
        UpdateMaterialConstantBuffer(PrimitiveData.Material.lock(), PrimitiveData);
        UpdateObjectConstantBuffer(PrimitiveData);

        HandleInputAssemblerStage(PrimitiveData);
        HandleVertexShaderStage(PrimitiveData);
        HandleGeometryShaderStage(PrimitiveData);
        HandlePixelShaderStage(PrimitiveData);
        HandleRasterizerStage(PrimitiveData);
        HandleOutputMergeStage(PrimitiveData);

        DrawPrimitive(PrimitiveData);
    }
}

void MRenderPass::Clear()
{
    if (RenderTargetViewData.empty() == false)
    {
        std::vector<ID3D11RenderTargetView*> RawRenderTargets;
        RawRenderTargets.reserve(RenderTargetViewData.size());
        for (const FRenderTargetBindData& BindData : RenderTargetViewData)
        {
            auto& RenderTarget = getRenderer()->GetRenderTarget(BindData.Index);
            assert(RenderTarget);

            RawRenderTargets.push_back(RenderTarget->AsRenderTargetView());

            if (true == bClearTargets)
            {
                g_pGraphicDevice->ClearRenderTarget(RenderTarget, Color);
            }
        }
    }
}

void MRenderPass::Begin()
{
	if (RenderTargetViewData.empty() == false)
	{
		std::vector<ID3D11RenderTargetView*> RenderTargetViews;
        RenderTargetViews.reserve(RenderTargetViewData.size());

        ID3D11DepthStencilView* DepthStencilView = bUseCommonDepthStencil ? g_pGraphicDevice->GetDepthStencilView() : nullptr;

        // RenderTargetViews
		for (const FRenderTargetBindData& BindData : RenderTargetViewData)
		{
            auto& RenderTarget = getRenderer()->GetRenderTarget(BindData.Index);
            assert(RenderTarget);

            if (RenderTarget->GetRenderTargetInfo().Type != ERenderTargetType::Depth)
            {
                RenderTargetViews.push_back(RenderTarget->AsRenderTargetView());
            }
            
            if(DepthStencilView == nullptr)
            {
                if (RenderTarget->GetRenderTargetInfo().Type == ERenderTargetType::Depth || RenderTarget->GetRenderTargetInfo().Type == ERenderTargetType::LinearDepth)
                {
                    DepthStencilView = RenderTarget->getDepthStencilView();
                }
            }

			if (bClearTargets)
			{
                g_pGraphicDevice->ClearRenderTarget(RenderTarget, Color);
			}
		}

        g_pGraphicDevice->getContext()->OMSetRenderTargets(static_cast<UINT>(RenderTargetViews.size()), RenderTargetViews.data(), DepthStencilView);
	}

    auto& ViewportSize = getGraphicDevice()->GetViewportSize();
    uint32 RectWidth = std::get<0>(ViewportSize);
    uint32 RectHeight = std::get<1>(ViewportSize);

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

void MRenderPass::DrawPrimitive(const FPrimitiveData& PrimitiveData)
{
    std::shared_ptr<MVertexBuffer>& VertexBuffer = PrimitiveData.VertexBuffer.lock();

    if (std::shared_ptr<MVertexBuffer> InstanceBuffer = PrimitiveData.InstanceBuffer.lock())
    {
        getGraphicDevice()->DrawInstance(PrimitiveData.VertexBuffer.lock(), PrimitiveData.IndexBuffer.lock(), InstanceBuffer);
    }
    else
    {
        getGraphicDevice()->Draw(PrimitiveData.VertexBuffer.lock(), PrimitiveData.IndexBuffer.lock());
    }
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

        if (bUseDefaultShaderOnly == false)
        {
            std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock();
            if (Material == nullptr)
            {
                return false;
            }
        }
    }

    //if (PrimitiveData.VertexBuffer.lock() == nullptr)
    //{
    //    return false;
    //}

    return true;
}

void MRenderPass::UpdateRenderPassConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    //// 기타 옵션들 자동으로 설정
    //for (auto& Prop : GetTypeDesc()->Properties)
    //{
    //    if (Prop->Type == EType::Bool)
    //    {
    //        bool bValue = *static_cast<bool*>(Prop->GetAsVoid(this));
    //        BOOL Value = bValue ? TRUE : FALSE;
    //        TickBuffer->SetData(StringToWString(Prop->Name), &Value);
    //    }
    //    else
    //    {
    //        TickBuffer->SetData(StringToWString(Prop->Name), Prop->GetAsVoid(this));
    //    }
    //}
}

void MRenderPass::UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    const auto& Camera = getRenderer()->GetWorld()->getMainCamera();
	const std::shared_ptr<MPrimitiveComponent>& PrimitiveComp = PrimitiveData.PrimitiveComponent.lock();

    std::shared_ptr<MShader>& VS = GetVertexShader(PrimitiveData);

    auto GetViewProjMatrix = [](bool bOrtho)->Mat4 {
        return bOrtho ? getRenderer()->ViewOrthogonalProjMatrix : getRenderer()->ViewPerspectiveProjMatrix;
    };

	// -------------------------------------------------------------------------------------------------------------------------
	// 버텍스쉐이더 ConstantBuffer
    if (VS->HasConstantBuffer(EConstantBufferLayer::Object))
    {
        BOOL animated = FALSE;
        Vec2 UV = { 1.f, 1.f };

        if (PrimitiveComp)
        {
            VS->SetValue(TEXT("worldMatrix"), PrimitiveComp->getWorldMatrix());

            Mat4 WorldView = {};
            XMStoreFloat4x4(&WorldView, XMLoadFloat4x4(&PrimitiveComp->getWorldMatrix()) * XMLoadFloat4x4(&Camera->getViewMatrix()));
            VS->SetValue(TEXT("WorldView"), WorldView);

            Mat4 WorldViewProj = {};
            XMStoreFloat4x4(&WorldViewProj, XMLoadFloat4x4(&PrimitiveComp->getWorldMatrix()) * XMLoadFloat4x4(&GetViewProjMatrix(PrimitiveComp->getRenderMdoe() == MPrimitiveComponent::ERenderMode::Orthogonal)));
            VS->SetValue(TEXT("WorldViewProj"), WorldViewProj);

            VS->SetValue(TEXT("inverseWorldMatrix"), PrimitiveComp->GetInverseWorldMatrix());
            VS->SetValue(TEXT("bOrtho"), PrimitiveComp->getRenderMdoe() == MPrimitiveComponent::ERenderMode::Orthogonal ? TRUE : FALSE);
            if (std::shared_ptr<DynamicMeshComponent> DynamicMeshComp = PrimitiveComp->CastToShared<DynamicMeshComponent>())
            {
                animated = DynamicMeshComp->HasAnim() && DynamicMeshComp->bBindPose == false ? TRUE : FALSE;
                if (animated)
                {
                    VS->SetValue(TEXT("keyFrameMatrices"), DynamicMeshComp->GetAnimMatrices());
                }
            }

            if (auto& Mat = PrimitiveData.Material.lock())
            {
                UV = Mat->UVScale;
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
            VS->SetValue(TEXT("bOrtho"), PrimitiveData.ProjectionType == EProjectionType::Orthograhpic ? TRUE : FALSE);
        }

        VS->SetValue(TEXT("animated"), animated);
        VS->SetValue(TEXT("bInstance"), PrimitiveData.InstanceBuffer.expired() == false ? TRUE : FALSE);

        VS->SetValue(TEXT("ScaleU"), UV.x);
        VS->SetValue(TEXT("ScaleV"), UV.y);
    }

    /*
	// -------------------------------------------------------------------------------------------------------------------------
	// 픽셀쉐이더 ConstantBuffer
    std::shared_ptr<MShader>& PS = GetPixelShader(PrimitiveData);
    if (PS->HasConstantBuffer(EConstantBufferLayer::Object))
    {
        if (std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock())
        {
            BOOL bUseNormal = Material->IsTextureTypeUsed(ETextureType::Normal) ? TRUE : FALSE;
            PS->SetValue(TEXT("bUseNormalTexture"), bUseNormal);
            BOOL bUseSpecular = Material->IsTextureTypeUsed(ETextureType::Specular) ? TRUE : FALSE;
            PS->SetValue(TEXT("bUseSpecularTexture"), bUseSpecular);
            BOOL bUseEmissive = Material->IsTextureTypeUsed(ETextureType::Emssive) ? TRUE : FALSE;
            PS->SetValue(TEXT("bUseEmissiveTexture"), bUseEmissive);
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
    */
}

void MRenderPass::UpdateMaterialConstantBuffer(std::shared_ptr<MMaterial>& InMaterial, const FPrimitiveData& PrimitiveData)
{
    BOOL bUseNormal = FALSE;
    BOOL bUseSpecular = FALSE;
    BOOL bUseEmissive = FALSE;
    BOOL bAlphaMask = FALSE;
    BOOL bRimLight = FALSE;
    Vec2 UVScale = { 1.f, 1.f };

    if (InMaterial)
    {
        bUseNormal = InMaterial->IsTextureTypeUsed(ETextureType::Normal) ? TRUE : FALSE;
        bUseSpecular = InMaterial->IsTextureTypeUsed(ETextureType::Specular) ? TRUE : FALSE;
        bUseEmissive = InMaterial->IsTextureTypeUsed(ETextureType::Emssive) ? TRUE : FALSE;
        bAlphaMask = InMaterial->IsAlphaMasked() ? TRUE : FALSE;
        bRimLight = InMaterial->IsRimLighted() ? TRUE : FALSE;
        UVScale = InMaterial->UVScale;
    }

    if (auto& PS = GetPixelShader(PrimitiveData))
    {
        PS->SetValue(TEXT("bUseNormalTexture"), bUseNormal);
        PS->SetValue(TEXT("bUseSpecularTexture"), bUseSpecular);
        PS->SetValue(TEXT("bUseEmissiveTexture"), bUseEmissive);
        PS->SetValue(TEXT("bAlphaMask"), bAlphaMask);
        PS->SetValue(TEXT("bRimLight"), bRimLight);
    }

    if (auto& VS = GetVertexShader(PrimitiveData))
    {
        VS->SetValue(TEXT("UVScale"), UVScale);
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
    //VertexShader->UpdateConstantBuffer(EConstantBufferLayer::Object);
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
        GeometryShader->Apply();
    }
}

void MRenderPass::HandlePixelShaderStage(const FPrimitiveData& PrimitiveData)
{
    std::shared_ptr<MShader> PixelShader = GetPixelShader(PrimitiveData);
    PixelShader->Apply();

    if (std::shared_ptr<MMaterial>& Material = PrimitiveData.Material.lock())
    {
        Material->SetTexturesToDevice();
    }

    if (bLikeMaterial)
    {
        for (uint32 i=0; i<GetSize(ResourceViewData); ++i)
        {
            const FRenderTargetBindData& Data = ResourceViewData[i];
            ID3D11ShaderResourceView* SRV = getRenderer()->GetResourceView(Data.Index);
            g_pGraphicDevice->getContext()->PSSetShaderResources(static_cast<UINT>(EnumToIndex(ETextureType::Diffuse) + i), 1, &SRV);
        }
    }
    else
    {
        for (const FRenderTargetBindData& Data : ResourceViewData)
        {
            ID3D11ShaderResourceView* SRV = getRenderer()->GetResourceView(Data.Index);
            g_pGraphicDevice->getContext()->PSSetShaderResources(static_cast<UINT>(Data.Index), 1, &SRV);
        }
    }

    OnHandlePxielShaderStage.Broadcast(PrimitiveData, PixelShader);
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

//void MRenderPass::SetUseOwningDepthStencilBuffer(const ERenderTarget bUse)
//{
//	UseOwningDepthStencilBuffer = bUse;
//}
//
