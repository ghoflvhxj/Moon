#include "RenderPass.h"

#include "MoonEngine.h"

#include "MainGameSetting.h"
#include "Renderer.h"


// Graphic
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"
#include "Module/Graphic/Shader/VertexShader.h"
#include "Module/Graphic/Shader/PixelShader.h"
#include "Module/Graphic/Shader/GeometryShader.h"

// Render
#include "RenderTarget.h"

// Game
#include "World.h"

// Actor
#include "Camera.h"

#include "Texture.h"

#include "DynamicMeshComponent.h"
#include "Framework/Component/FX/FXComponent.h"

using namespace DirectX;

MRenderPass::MRenderPass()
	: _vertexShader{ nullptr }
	, _pixelShader{ nullptr }
	, GeometryShader{ nullptr }
	, _bShaderSet{ false }
{
}

MRenderPass::~MRenderPass()
{
}

void MRenderPass::RenderPass(std::vector<FPrimitiveData>& PrimitiveDatList)
{ 
    if (_vertexShader && _vertexShader->HasConstantBuffer(EConstantBufferLayer::RenderPass))
    {
        UpdateRenderPassConstantBuffer(_vertexShader);
    }

    if (_pixelShader && _pixelShader->HasConstantBuffer(EConstantBufferLayer::RenderPass))
    {
        UpdateRenderPassConstantBuffer(_pixelShader);
    }

    for (auto& PrimitiveData : PrimitiveDatList)
    {
        if (IsValidPrimitive(PrimitiveData) == false)
        {
            continue;
        }

        HandleInputAssemblerStage(PrimitiveData);

        if (ComputeShader.IsValid())
        {
            HandleComputeShaderStage(&ComputeShader, PrimitiveData);
            getGraphicDevice()->CSSet(ComputeShader);
        }
        else
        {
            getGraphicDevice()->CSReset();
        }

        if (std::shared_ptr<MVertexShader> VS = GetVertexShader(PrimitiveData))
        {
            HandleVertexShaderStage(VS, PrimitiveData);
            VS->Apply();
        }
        else
        {
            assert(false); // VS는 반드시 세팅되야 함
        }

        if (std::shared_ptr<MGeometryShader> GS = GetGeometryShader(PrimitiveData))
        {
            HandleGeometryShaderStage(GS, PrimitiveData);
            GS->Apply();
        }
        else
        {
            getGraphicDevice()->GSReset();
        }

        if (std::shared_ptr<MPixelShader> PS = GetPixelShader(PrimitiveData))
        {
            HandlePixelShaderStage(PS, PrimitiveData);
            PS->Apply();
            OnHandlePxielShaderStage.Broadcast(PrimitiveData, PS);
        }
        else
        {
            getGraphicDevice()->PSSet(nullptr);
        }

        HandleRasterizerStage(PrimitiveData);
        HandleOutputMergeStage(PrimitiveData);

        DrawPrimitive(PrimitiveData);

        if (std::shared_ptr<MVertexShader> VS = GetVertexShader(PrimitiveData))
        {
            if (ComputeShader.IsValid())
            {
                getGraphicDevice()->VSSetSRV(ComputeShader.RWStructuredBuffer.GetSlot(), -1);
            }
        }
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
    getGraphicDevice()->SetToDefault();
}

void MRenderPass::DrawPrimitive(const FPrimitiveData& PrimitiveData)
{
    std::shared_ptr<MVertexBuffer>& VertexBuffer = PrimitiveData.VertexBuffer.lock();

    if (std::shared_ptr<MVertexBuffer> InstanceBuffer = PrimitiveData.InstanceBuffer.lock())
    {
        getGraphicDevice()->DrawInstance(PrimitiveData.VertexBuffer.lock(), PrimitiveData.IndexBuffer.lock(), InstanceBuffer);
    }
    else if (PrimitiveData.InstanceNum > 0)
    {
        getGraphicDevice()->DrawInstance(PrimitiveData.VertexBuffer.lock(), PrimitiveData.IndexBuffer.lock(), PrimitiveData.InstanceNum);
    }
    else
    {
        getGraphicDevice()->Draw(PrimitiveData.VertexBuffer.lock(), PrimitiveData.IndexBuffer.lock());
    }
}

bool MRenderPass::IsValidPrimitive(const FPrimitiveData& InPrimitiveData) const
{
    const std::shared_ptr<MPrimitiveComponent>& Primitive = InPrimitiveData.PrimitiveComponent.lock();
    if (Primitive != nullptr)
    {
        if (Primitive->IsRendering() == false)
        {
            return false;
        }

        if (bUseDefaultShaderOnly == false)
        {
            std::shared_ptr<MMaterial>& Material = InPrimitiveData.Material.lock();
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

void MRenderPass::UpdateRenderPassConstantBuffer(std::shared_ptr<MShader> InShader)
{
    assert(InShader);

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

void MRenderPass::UpdateRenderPassObjectConstantBuffer(std::shared_ptr<MShader> InShader, const FPrimitiveData& PrimitiveData)
{
    assert(InShader);
}

void MRenderPass::UpdateObjectConstantBuffer(std::shared_ptr<MShader> InShader, const FPrimitiveData& PrimitiveData)
{
    assert(InShader);

    const auto& Camera = getRenderer()->GetWorld()->getMainCamera();
	const std::shared_ptr<MPrimitiveComponent>& PrimitiveComp = PrimitiveData.PrimitiveComponent.lock();

    auto GetViewProjMatrix = [](bool bOrtho)->Mat4 {
        return bOrtho ? getRenderer()->ViewOrthogonalProjMatrix : getRenderer()->ViewPerspectiveProjMatrix;
    };


    bool animated = false;
    Vec2 UV = { 1.f, 1.f };

    if (PrimitiveComp && PrimitiveData.bUseCustomTransform == false)
    {
        InShader->SetValue(TEXT("worldMatrix"), PrimitiveComp->getWorldMatrix());

        Mat4 WorldView = {};
        XMStoreFloat4x4(&WorldView, XMLoadFloat4x4(&PrimitiveComp->getWorldMatrix()) * XMLoadFloat4x4(&Camera->getViewMatrix()));
        InShader->SetValue(TEXT("WorldView"), WorldView);

        Mat4 WorldViewProj = {};
        XMStoreFloat4x4(&WorldViewProj, XMLoadFloat4x4(&PrimitiveComp->getWorldMatrix()) * XMLoadFloat4x4(&GetViewProjMatrix(PrimitiveComp->getRenderMdoe() == MPrimitiveComponent::ERenderMode::Orthogonal)));
        InShader->SetValue(TEXT("WorldViewProj"), WorldViewProj);

        InShader->SetValue(TEXT("InverseWorldMatrix"), PrimitiveComp->GetInverseWorldMatrix());
        InShader->SetValue(TEXT("bOrtho"), PrimitiveComp->getRenderMdoe() == MPrimitiveComponent::ERenderMode::Orthogonal);
        if (std::shared_ptr<DynamicMeshComponent> DynamicMeshComp = PrimitiveComp->CastToShared<DynamicMeshComponent>())
        {
            animated = DynamicMeshComp->HasAnim() && DynamicMeshComp->bBindPose == false;
            if (animated)
            {
                InShader->SetValue(TEXT("keyFrameMatrices"), DynamicMeshComp->GetAnimMatrices());
            }
        }
    }
    else
    {
        Mat4 WorldMatrix = {};
        XMMATRIX XMWorldMat = XMMatrixScalingFromVector(XMLoadFloat3(&PrimitiveData.Scale)) * XMMatrixRotationQuaternion(XMLoadFloat4(&PrimitiveData.Rotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&PrimitiveData.Translation));
        XMStoreFloat4x4(&WorldMatrix, XMWorldMat);
        InShader->SetValue(TEXT("worldMatrix"), WorldMatrix);

        Mat4 WorldView = {};
        XMStoreFloat4x4(&WorldView, XMWorldMat * XMLoadFloat4x4(&Camera->getViewMatrix()));
        InShader->SetValue(TEXT("WorldView"), WorldView);

        Mat4 WorldViewProj = {};
        XMStoreFloat4x4(&WorldViewProj, XMWorldMat * XMLoadFloat4x4(&GetViewProjMatrix(PrimitiveData.ProjectionType == EProjectionType::Orthograhpic)));
        InShader->SetValue(TEXT("WorldViewProj"), WorldViewProj);

        Mat4 InvWorldMatrix = {};
        XMStoreFloat4x4(&InvWorldMatrix, XMMatrixInverse(nullptr, XMWorldMat));
        InShader->SetValue(TEXT("InverseWorldMatrix"), InvWorldMatrix);
        InShader->SetValue(TEXT("bOrtho"), PrimitiveData.ProjectionType == EProjectionType::Orthograhpic);
    }

    InShader->SetValue(TEXT("animated"), animated);
    InShader->SetValue(TEXT("bInstance"), PrimitiveData.InstanceBuffer.expired() == false);
}

void MRenderPass::UpdateMaterialConstantBuffer(std::shared_ptr<MShader> InShader, std::shared_ptr<MMaterial>& InMaterial, const FPrimitiveData& PrimitiveData)
{
    assert(InShader);

    if (InShader->IsPixelShader())
    {
        bool bUseNormal = false;
        bool bUseSpecular = false;
        bool bUseEmissive = false;
        bool bAlphaMask = false;
        bool bRimLight = false;

        if (InMaterial)
        {
            bUseNormal = InMaterial->IsTextureTypeUsed(ETextureType::Normal);
            bUseSpecular = InMaterial->IsTextureTypeUsed(ETextureType::Specular);
            bUseEmissive = InMaterial->IsTextureTypeUsed(ETextureType::Emssive);
            bAlphaMask = InMaterial->IsAlphaMasked();
            bRimLight = InMaterial->IsRimLighted();
        }

        InShader->SetValue(TEXT("bUseNormalTexture"), bUseNormal);
        InShader->SetValue(TEXT("bUseSpecularTexture"), bUseSpecular);
        InShader->SetValue(TEXT("bUseEmissiveTexture"), bUseEmissive);
        InShader->SetValue(TEXT("bAlphaMask"), bAlphaMask);
        InShader->SetValue(TEXT("bRimLight"), bRimLight);
    }

    if (InShader->IsVertexShader())
    {
        Vec2 UVScale = { 1.f, 1.f };
        if (InMaterial)
        {
            UVScale = InMaterial->UVScale;
        }
        InShader->SetValue(TEXT("UVScale"), UVScale);
    }
}

void MRenderPass::UpdateStructuredBuffer(std::shared_ptr<MShader> InShader, const FPrimitiveData& InPrimitiveData)
{
    // FX는 Structured 버퍼 초기 세팅만 해주고, 업데이트는 GPU 쪽에서 다 해줘야 함
    if (auto FXComp = InPrimitiveData.GetPrimitiveComponent<MFXComponent>())
    {
        UINT StructSize = static_cast<UINT>(sizeof(FParticle));
        UINT Num = static_cast<UINT>(GetSize(FXComp->Particles));

        if (InShader->StructuredBuffer.GetBufferSize() != StructSize * Num)
        {
            getGraphicDevice()->UpdateStructuredBuffer(InShader->StructuredBuffer, FXComp->Particles.data(), StructSize * Num, Num, StructSize);
        }
    }
}

void MRenderPass::HandleInputAssemblerStage(const FPrimitiveData& PrimitiveData)
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    std::vector<ID3D11Buffer*> VertexBuffers(2, nullptr);
    std::vector<UINT> Strides(2, 0);
    std::vector<UINT> Offsets(2, 0);

    /********************************
        버텍스 버퍼 슬롯
         0: VertexBuffer
         1: InstanceBufer
    ********************************/

    // 버텍스 버퍼
    std::shared_ptr<MVertexBuffer>& VertexBuffer = PrimitiveData.VertexBuffer.lock();
    VertexBuffers[0] = VertexBuffer->getBuffer();
    Strides[0] = VertexBuffer->GetVertexSize();
    Offsets[0] = 0;

    // 인스턴싱 버퍼
    if (std::shared_ptr<MVertexBuffer>& InstanceBuffer = PrimitiveData.InstanceBuffer.lock())
    {
        VertexBuffers[1] = InstanceBuffer->getBuffer();
        Strides[1] = InstanceBuffer->GetVertexSize();
        Offsets[1] = 0;
    }

    UINT VertexBufferNum = GetSize(VertexBuffers);
    getGraphicDevice()->getContext()->IASetVertexBuffers(0, VertexBufferNum, VertexBuffers.data(), Strides.data(), Offsets.data());

    // 인덱스 버퍼
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

void MRenderPass::HandleVertexShaderStage(std::shared_ptr<MVertexShader> InVertexShader, const FPrimitiveData& PrimitiveData)
{
    assert(InVertexShader);

    if (InVertexShader->HasConstantBuffer(EConstantBufferLayer::RenderPassObject))
    {
        UpdateRenderPassObjectConstantBuffer(InVertexShader, PrimitiveData);
    }

    if (InVertexShader->HasConstantBuffer(EConstantBufferLayer::Material))
    {
        UpdateMaterialConstantBuffer(InVertexShader, PrimitiveData.Material.lock(), PrimitiveData);
    }

    if (InVertexShader->HasConstantBuffer(EConstantBufferLayer::Object))
    {
        UpdateObjectConstantBuffer(InVertexShader, PrimitiveData);
    }

    if (ComputeShader.IsValid() && ComputeShader.RWStructuredBuffer.IsValid())
    {
        //UpdateStructuredBuffer(InVertexShader, PrimitiveData);
        getGraphicDevice()->CSSetUAV(ComputeShader.RWStructuredBuffer.GetSlot(), -1);
        getGraphicDevice()->VSSetSRV(ComputeShader.RWStructuredBuffer);
    }
}

void MRenderPass::HandleGeometryShaderStage(std::shared_ptr<MGeometryShader> InGeometryShader, const FPrimitiveData& PrimitiveData)
{
    assert(InGeometryShader);
}

void MRenderPass::HandlePixelShaderStage(std::shared_ptr<MPixelShader> InPixelShader, const FPrimitiveData& InPrimitiveData)
{
    assert(InPixelShader);

    if (std::shared_ptr<MMaterial>& Material = InPrimitiveData.Material.lock())
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

    if (InPixelShader->HasConstantBuffer(EConstantBufferLayer::RenderPassObject))
    {
        UpdateRenderPassObjectConstantBuffer(InPixelShader, InPrimitiveData);
    }

    if (InPixelShader->HasConstantBuffer(EConstantBufferLayer::Material))
    {
        UpdateMaterialConstantBuffer(InPixelShader, InPrimitiveData.Material.lock(), InPrimitiveData);
    }

    if (InPixelShader->HasConstantBuffer(EConstantBufferLayer::Object))
    {
        UpdateObjectConstantBuffer(InPixelShader, InPrimitiveData);
    }

    //if (InPixelShader->RWStructuredBuffer.IsValid())
    //{
    //    getGraphicDevice()->PSSetSRV(InPixelShader->RWStructuredBuffer);
    //}
}

void MRenderPass::HandleComputeShaderStage(MComputeShader* InComputeShader, FPrimitiveData& InPrimitiveData)
{
    assert(InComputeShader);

    // ComputeShader에서는 GPU로 계산이 이뤄지도록 세팅만 해주면 됨
    
    if (InComputeShader->RWStructuredBuffer.IsValid())
    {
        // FX는 Structured 버퍼 초기 세팅만 해주고, 업데이트는 GPU 쪽에서 다 해줘야 함
        if (auto FXComp = InPrimitiveData.GetPrimitiveComponent<MFXComponent>())
        {
            UINT StructSize = static_cast<UINT>(sizeof(FParticle));
            UINT Num = static_cast<UINT>(GetSize(FXComp->Particles));

            if (Num == 0)
            {
                return;
            }

            if (InComputeShader->RWStructuredBuffer.GetBufferSize() != StructSize * Num)
            {
                getGraphicDevice()->UpdateStructuredBuffer(InComputeShader->RWStructuredBuffer, FXComp->Particles.data(), StructSize * Num, Num, StructSize);
            }

            InPrimitiveData.InstanceNum = GetSize(FXComp->Particles);
        }

        getGraphicDevice()->CSSetUAV(InComputeShader->RWStructuredBuffer);
    }

    //getGraphicDevice()->CSSetSRV(InComputeShader->RWStructuredBuffer);
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

std::shared_ptr<MVertexShader> MRenderPass::GetVertexShader(const FPrimitiveData& InPrimitiveData)
{
    std::shared_ptr<MMaterial>& Material = InPrimitiveData.Material.lock();
    std::shared_ptr<MVertexShader> OutShader = nullptr;
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

std::shared_ptr<MPixelShader> MRenderPass::GetPixelShader(const FPrimitiveData& InPrimitiveData)
{
    std::shared_ptr<MMaterial>& Material = InPrimitiveData.Material.lock();
    std::shared_ptr<MPixelShader> OutShader = nullptr;
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

std::shared_ptr<MGeometryShader> MRenderPass::GetGeometryShader(const FPrimitiveData& InPrimitiveData)
{
    return GeometryShader;
}

void MRenderPass::SetDefaultShader(EShaderType InShaderType, const std::wstring& InFileName)
{
    //g_pGraphicDevice->GetGeometryShader()
    switch (InShaderType)
    {
        case EShaderType::Vertex:
        {

        }
        break;
        case EShaderType::Pixel:
        {

        }
        break;
        case EShaderType::Geometry:
        {

        }
        break;
        case EShaderType::Compute:
        {
            getGraphicDevice()->GetComputeShader(InFileName, ComputeShader);
        }
        break;
    }
}

void MRenderPass::SetDefaultShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName)
{
	releaseShader();
	std::shared_ptr<MVertexShader> vertexShader = nullptr;
	if (g_pGraphicDevice->GetVertexShader(vertexShaderFileName, vertexShader))
	{
		_vertexShader = vertexShader;
		_vertexShaderFileName = vertexShaderFileName;;
	}
	
	std::shared_ptr<MPixelShader> pixelShader = nullptr;
	if (pixelShaderFileName != nullptr && g_pGraphicDevice->GetPixelShader(pixelShaderFileName, pixelShader))
	{
		_pixelShader = pixelShader;
		_pixelShaderFileName = pixelShaderFileName;
	}
	else
	{
		_pixelShader = std::make_shared<MPixelShader>();
	}

	_bShaderSet = true;
}

void MRenderPass::SetDefaultShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName, const wchar_t *geomtryShaderFileName)
{
	SetDefaultShader(vertexShaderFileName, pixelShaderFileName);

	std::shared_ptr<MGeometryShader> geometryShader = nullptr;
	if (g_pGraphicDevice->GetGeometryShader(geomtryShaderFileName, geometryShader))
	{
		GeometryShader = geometryShader;
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
