#include "Include.h"
#include "RenderTarget.h"

#include "GraphicDevice.h"
#include "Material.h"

#include "MeshComponent.h"
#include "Texture.h"

#include "MainGameSetting.h"

/**************************************
    TODO. DX 관련 코드는 GraphicDevice로 옮겨야 함.
**************************************/

MRenderTarget::MRenderTarget()
	: _pRenderTargetView{ nullptr }
	, _pDepthStencilView{ nullptr }
{
	//initializeTexture(FRenderTagetInfo::GetDefault());
}

MRenderTarget::~MRenderTarget()
{
	SafeRelease(_pRenderTargetView);
	SafeRelease(_pDepthStencilView);
}

std::shared_ptr<MTexture> MRenderTarget::AsTexture()
{
	return RenderTargetTexture ? RenderTargetTexture : DepthStencilTexture;
}

void MRenderTarget::initializeTexture(const FRenderTagetInfo& InRenderTargetInfo)
{
    RenderTargetInfo = InRenderTargetInfo;

	bool bNotDepth = RenderTargetInfo.Type != ERenderTargetType::Depth;
	bool bSingleTexture = RenderTargetInfo.bCube == false && RenderTargetInfo.TextrueNum == 1;
    constexpr uint32 CubeTexNum = 6;
    uint32 CubeNum = RenderTargetInfo.TextrueNum;
    uint32 TextureNum = RenderTargetInfo.bCube ? CubeTexNum * RenderTargetInfo.TextrueNum : RenderTargetInfo.TextrueNum;

    // 깊이만 기록한다면 렌더 타겟 뷰는 필요가 없음. 깊이 스텐실 뷰만 있으면 됨
    if (bNotDepth)
    {
        // 렌더 타겟 텍스쳐
        D3D11_TEXTURE2D_DESC TextureDesc = {};
        TextureDesc.Width = RenderTargetInfo.Width;
        TextureDesc.Height = RenderTargetInfo.Height;
        TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        TextureDesc.SampleDesc.Count = 1;
        TextureDesc.SampleDesc.Quality = 0;
        TextureDesc.ArraySize = TextureNum;
        TextureDesc.MipLevels = 1;
        TextureDesc.Usage = D3D11_USAGE_DEFAULT;
        TextureDesc.Format = GetFormat(EDXResourceType::Texture, RenderTargetInfo.Type);
        TextureDesc.CPUAccessFlags = 0;
        TextureDesc.MiscFlags = RenderTargetInfo.bCube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

        RenderTargetTexture = std::make_shared<MTexture>();

        ID3D11Texture2D* NewTexture = nullptr;
        //g_pGraphicDevice->getDevice()->CreateTexture2D(&TextureDesc, nullptr, &NewTexture);
        FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateTexture2D(&TextureDesc, nullptr, &NewTexture));
        RenderTargetTexture->SetTexture(NewTexture);

        // RenderTarget 렌더 타겟 뷰
        SafeRelease(_pRenderTargetView);
        D3D11_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc = { };
        RenderTargetViewDesc.Format = GetFormat(EDXResourceType::ShaderResourceView, RenderTargetInfo.Type);
        if (bSingleTexture)
        {
            RenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            RenderTargetViewDesc.Texture2D.MipSlice = 0;
        }
        else if (RenderTargetInfo.bCube)
        {
            RenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            RenderTargetViewDesc.Texture2DArray.ArraySize = TextureNum;
            RenderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;
            RenderTargetViewDesc.Texture2DArray.MipSlice = 0;
        }
        else
        {
            RenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            RenderTargetViewDesc.Texture2DArray.ArraySize = TextureNum;
            RenderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;
            RenderTargetViewDesc.Texture2DArray.MipSlice = 0;
        }
        FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateRenderTargetView(RenderTargetTexture->GetTexture(), &RenderTargetViewDesc, &_pRenderTargetView));

        // RenderTarget 쉐이더 리소스 뷰
        D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc = { };
        ShaderResourceViewDesc.Format = GetFormat(EDXResourceType::ShaderResourceView, RenderTargetInfo.Type);
        ShaderResourceViewDesc.ViewDimension = bSingleTexture ? D3D11_SRV_DIMENSION_TEXTURE2D : RenderTargetInfo.bCube ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        if (bSingleTexture)
        {
            ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            ShaderResourceViewDesc.Texture2D.MipLevels = -1;
            ShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
        }
        else if (RenderTargetInfo.bCube)
        {
            if (TextureNum == 6)
            {
                ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
                ShaderResourceViewDesc.TextureCube.MipLevels = -1;
                ShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
            }
            else
            {
                ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
                ShaderResourceViewDesc.TextureCubeArray.NumCubes = CubeNum;
                ShaderResourceViewDesc.TextureCubeArray.MipLevels = -1;
                ShaderResourceViewDesc.TextureCubeArray.MostDetailedMip = 0;
                ShaderResourceViewDesc.TextureCubeArray.First2DArrayFace = 0;
            }
        }
        else
        {
            ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            ShaderResourceViewDesc.Texture2DArray.ArraySize = TextureNum;
            ShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
            ShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
            ShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
        }

        //FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateShaderResourceView(RenderTargetTexture->GetTexture(), &ShaderResourceViewDesc, &RenderTargetTexture->GetShaderResourceView()));
        ID3D11ShaderResourceView* ShaderResourceView = nullptr;
        FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateShaderResourceView(RenderTargetTexture->GetTexture(), &ShaderResourceViewDesc, &ShaderResourceView));
        RenderTargetTexture->SetShaderResourceView(ShaderResourceView);
    }
    
    if(bNotDepth == false || RenderTargetInfo.Type == ERenderTargetType::LinearDepth)
    {
        // DepthStencil 텍스쳐
        D3D11_TEXTURE2D_DESC DepthStencilTextureDesc = { };
        DepthStencilTextureDesc.Width = RenderTargetInfo.Width;
        DepthStencilTextureDesc.Height = RenderTargetInfo.Height;
        DepthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        DepthStencilTextureDesc.SampleDesc.Count = 1;
        DepthStencilTextureDesc.SampleDesc.Quality = 0;
        DepthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        DepthStencilTextureDesc.Format = GetDepthStencilFormat(EDXResourceType::Texture, RenderTargetInfo.Type);
        DepthStencilTextureDesc.ArraySize = TextureNum;
        DepthStencilTextureDesc.MipLevels = 1;
        DepthStencilTextureDesc.CPUAccessFlags = 0;
        DepthStencilTextureDesc.MiscFlags = RenderTargetInfo.bCube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
        DepthStencilTexture = std::make_shared<MTexture>();

        //FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateTexture2D(&DepthStencilTextureDesc, nullptr, &DepthStencilTexture->GetTexture()));
        ID3D11Texture2D* DepthTexture = nullptr;
        g_pGraphicDevice->getDevice()->CreateTexture2D(&DepthStencilTextureDesc, nullptr, &DepthTexture);
        DepthStencilTexture->SetTexture(DepthTexture);

        // DepthStencil 깊이 스텐실 뷰
        SafeRelease(_pDepthStencilView);
        D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
        DepthStencilViewDesc.Format = GetDepthStencilFormat(EDXResourceType::RenderTargetView, RenderTargetInfo.Type);
        if (bSingleTexture)
        {
            DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            DepthStencilViewDesc.Texture2D.MipSlice = 0;
        }
        else if (RenderTargetInfo.bCube)
        {
            DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            DepthStencilViewDesc.Texture2DArray.ArraySize = TextureNum;
            DepthStencilViewDesc.Texture2DArray.FirstArraySlice = 0;
            DepthStencilViewDesc.Texture2DArray.MipSlice = 0;
        }
        else
        {
            DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            DepthStencilViewDesc.Texture2DArray.ArraySize = TextureNum;
            DepthStencilViewDesc.Texture2DArray.FirstArraySlice = 0;
            DepthStencilViewDesc.Texture2DArray.MipSlice = 0;
        }
        FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateDepthStencilView(DepthTexture, &DepthStencilViewDesc, &_pDepthStencilView));

        // 깊이 쉐이더 리소스 뷰
        D3D11_SHADER_RESOURCE_VIEW_DESC DepthResourceViewDesc = { };
        DepthResourceViewDesc.Format = GetDepthStencilFormat(EDXResourceType::ShaderResourceView, RenderTargetInfo.Type);
        if (bSingleTexture)
        {
            DepthResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            DepthResourceViewDesc.Texture2D.MostDetailedMip = 0;
            DepthResourceViewDesc.Texture2D.MipLevels = -1;
        }
        else if (RenderTargetInfo.bCube)
        {
            DepthResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
            DepthResourceViewDesc.TextureCubeArray.MostDetailedMip = 0;
            DepthResourceViewDesc.TextureCubeArray.MipLevels = -1;
            DepthResourceViewDesc.TextureCubeArray.First2DArrayFace = 0;
            DepthResourceViewDesc.TextureCubeArray.NumCubes = CubeNum;
        }
        else
        {
            DepthResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            DepthResourceViewDesc.Texture2DArray.ArraySize = TextureNum;
            DepthResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
            DepthResourceViewDesc.Texture2DArray.MipLevels = -1;
            DepthResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
        }

        ID3D11ShaderResourceView* ShaderResourceView = nullptr;
        FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateShaderResourceView(DepthTexture, &DepthResourceViewDesc, &ShaderResourceView));
        DepthStencilTexture->SetShaderResourceView(ShaderResourceView);
    }
}

void MRenderTarget::UpdateResolution(float InWidth, float InHeight)
{
    //if (RenderTargetInfo.bCube == false)
    //{
    //    RenderTargetInfo.Width = InWidth;
    //    RenderTargetInfo.Height = InHeight;

    //    initializeTexture(RenderTargetInfo);
    //}
}

DXGI_FORMAT MRenderTarget::GetFormat(EDXResourceType InViewType, ERenderTargetType InRenderTargetType) const
{
    // 뎁스라면 텍스쳐 -> R24G8, 리소스 ->  R24X8
    if (InViewType == EDXResourceType::Texture)
    {
        switch (InRenderTargetType)
        {
        case ERenderTargetType::Default:
        case ERenderTargetType::Diffuse:
            return DXGI_FORMAT_R8G8B8A8_TYPELESS;
        case ERenderTargetType::Normal:
            return DXGI_FORMAT_R10G10B10A2_UNORM;
        case ERenderTargetType::Light:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case ERenderTargetType::Depth:
            return DXGI_FORMAT_R24G8_TYPELESS;
        case ERenderTargetType::LinearDepth:
            return DXGI_FORMAT_R32_TYPELESS;
        case ERenderTargetType::Bool:
            return DXGI_FORMAT_R8_TYPELESS;
        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }
    //else if (InViewType == EDXResourceType::RenderTargetView)
    //{
    //    switch (InRenderTargetType)
    //    {
    //    case ERenderTargetType::Default:
    //        return DXGI_FORMAT_R8G8B8A8_UNORM;
    //    case ERenderTargetType::Diffuse:
    //        return DXGI_FORMAT_R8G8B8A8_UNORM;
    //    case ERenderTargetType::Normal:
    //        return DXGI_FORMAT_R10G10B10A2_UNORM;
    //    case ERenderTargetType::Light:
    //        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    //    case ERenderTargetType::Depth:
    //        return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    //    case ERenderTargetType::Bool:
    //        return DXGI_FORMAT_R8_UNORM;
    //    default:
    //        return DXGI_FORMAT_UNKNOWN;
    //    }
    //}
    else if (InViewType == EDXResourceType::ShaderResourceView)
    {
        switch (InRenderTargetType)
        {
        case ERenderTargetType::Default:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case ERenderTargetType::Diffuse:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case ERenderTargetType::Normal:
            return DXGI_FORMAT_R10G10B10A2_UNORM;
        case ERenderTargetType::Light:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case ERenderTargetType::Depth:
            return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        case ERenderTargetType::LinearDepth:
            return DXGI_FORMAT_R32_FLOAT;
        case ERenderTargetType::Bool:
            return DXGI_FORMAT_R8_UNORM;
        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }
}

DXGI_FORMAT MRenderTarget::GetDepthStencilFormat(EDXResourceType InViewType, ERenderTargetType InRenderTargetType) const
{
    if (InViewType == EDXResourceType::Texture)
    {
        switch (InRenderTargetType)
        {
        case ERenderTargetType::Depth:
            return DXGI_FORMAT_R24G8_TYPELESS;
        case ERenderTargetType::LinearDepth:
            return DXGI_FORMAT_R32_TYPELESS;
        }
    }
    else if(InViewType == EDXResourceType::RenderTargetView)
    {
        switch (InRenderTargetType)
        {
        case ERenderTargetType::Depth:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case ERenderTargetType::LinearDepth:
            return DXGI_FORMAT_D32_FLOAT;
        }
    }
    else if (InViewType == EDXResourceType::ShaderResourceView)
    {
        switch (InRenderTargetType)
        {
        case ERenderTargetType::Depth:
            return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        case ERenderTargetType::LinearDepth:
            return DXGI_FORMAT_R32_FLOAT;
        }
    }

    return DXGI_FORMAT_UNKNOWN;
}

ID3D11RenderTargetView* MRenderTarget::AsRenderTargetView()
{
	return _pRenderTargetView;
}

ID3D11DepthStencilView* MRenderTarget::getDepthStencilView()
{
	return _pDepthStencilView;
}

const FRenderTagetInfo FRenderTagetInfo::GetDefault(uint32 InWidth, uint32 InHeight)
{
	FRenderTagetInfo RenderTargetInfo = {};
	RenderTargetInfo.bCube = false;
	RenderTargetInfo.TextrueNum = 1;
	RenderTargetInfo.Width = InWidth;
	RenderTargetInfo.Height = InHeight;
    RenderTargetInfo.Type = ERenderTargetType::Default;

	return RenderTargetInfo;
}

const FRenderTagetInfo FRenderTagetInfo::GetCube()
{
	FRenderTagetInfo RenderTargetInfo;
    RenderTargetInfo.bCube = true;
    RenderTargetInfo.TextrueNum = 1;
	RenderTargetInfo.Width = 2048;	// 쉐도우맵 해상도
	RenderTargetInfo.Height = 2048;
    RenderTargetInfo.Type = ERenderTargetType::Default;

	return RenderTargetInfo;
}
