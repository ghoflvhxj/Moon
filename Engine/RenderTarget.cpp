#include "Include.h"
#include "RenderTarget.h"

#include "GraphicDevice.h"
#include "Material.h"

#include "MeshComponent.h"
#include "Texture.h"

#include "MainGame.h"
#include "MainGameSetting.h"

RenderTarget::RenderTarget()
	: RenderTargetTexture{ nullptr }
	, _pRenderTargetView{ nullptr }
	, DepthStencilTexture{ nullptr }
	, _pDepthStencilView{ nullptr }
{
	initializeTexture(FRenderTagetInfo::GetDefault());
}

RenderTarget::RenderTarget(const FRenderTagetInfo& RenderTargetInfo)
	: RenderTargetTexture{ nullptr }
	, _pRenderTargetView{ nullptr }
	, DepthStencilTexture{ nullptr }
	, _pDepthStencilView{ nullptr }
{
	initializeTexture(RenderTargetInfo);
}

RenderTarget::~RenderTarget()
{
	SafeRelease(_pRenderTargetView);
	SafeRelease(_pDepthStencilView);
}

std::shared_ptr<MTexture> RenderTarget::AsTexture()
{
	return RenderTargetTexture;
}

void RenderTarget::initializeTexture(const FRenderTagetInfo& RenderTargetInfo)
{
	bool bDepthOnly = RenderTargetInfo.TextrueNum == 1;
	bool bSingleTexture = RenderTargetInfo.TextrueNum <= 1;

	// RenderTarget ÅØ½ºÃ³
	D3D11_TEXTURE2D_DESC TextureDesc = {};
	TextureDesc.Width = RenderTargetInfo.Width;
	TextureDesc.Height = RenderTargetInfo.Height;
	TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.ArraySize = RenderTargetInfo.TextrueNum;
	TextureDesc.MipLevels = 1;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.Format = bDepthOnly ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R32_TYPELESS;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = RenderTargetInfo.bCube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

	RenderTargetTexture = std::make_shared<MTexture>();
	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateTexture2D(&TextureDesc, nullptr, &RenderTargetTexture->GetTextureResource()));

	// RenderTarget ·»´õ Å¸°Ù ºä
	D3D11_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc		= { };
	RenderTargetViewDesc.Format								= bDepthOnly ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R32_FLOAT;
	if (bSingleTexture)
	{
		RenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		RenderTargetViewDesc.Texture2D.MipSlice = 0;
	}
	else if(RenderTargetInfo.bCube)
	{
		RenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		RenderTargetViewDesc.Texture2DArray.ArraySize = 6;
		RenderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;
		RenderTargetViewDesc.Texture2DArray.MipSlice = 0;
	}
	else
	{
		RenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		RenderTargetViewDesc.Texture2DArray.ArraySize = RenderTargetInfo.TextrueNum;
		RenderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;
		RenderTargetViewDesc.Texture2DArray.MipSlice = 0;
	}
	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateRenderTargetView(RenderTargetTexture->GetTextureResource(), &RenderTargetViewDesc, &_pRenderTargetView));

	// RenderTarget ½¦ÀÌ´õ ¸®¼Ò½º ºä
	D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc	= { };
	ShaderResourceViewDesc.Format							= bDepthOnly ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R32_FLOAT;
	ShaderResourceViewDesc.ViewDimension					= bSingleTexture ? D3D11_SRV_DIMENSION_TEXTURE2D : RenderTargetInfo.bCube ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	if (bSingleTexture)
	{
		ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		ShaderResourceViewDesc.Texture2D.MipLevels = -1;
		ShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	}
	else if (RenderTargetInfo.bCube)
	{
		ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		ShaderResourceViewDesc.TextureCube.MipLevels = -1;
		ShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
	}
	else
	{
		ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		ShaderResourceViewDesc.Texture2DArray.ArraySize = RenderTargetInfo.TextrueNum;
		ShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		ShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		ShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
	}

	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateShaderResourceView(RenderTargetTexture->GetTextureResource(), &ShaderResourceViewDesc, &RenderTargetTexture->getRawResourceViewPointer()));

	// DepthStencil ÅØ½ºÃÄ
	D3D11_TEXTURE2D_DESC DepthStencilTextureDesc = { };
	DepthStencilTextureDesc.Width = RenderTargetInfo.Width;
	DepthStencilTextureDesc.Height = RenderTargetInfo.Height;
	DepthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	DepthStencilTextureDesc.SampleDesc.Count = 1;
	DepthStencilTextureDesc.SampleDesc.Quality = 0;
	DepthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	DepthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthStencilTextureDesc.ArraySize = RenderTargetInfo.TextrueNum;
	DepthStencilTextureDesc.MipLevels = 1;
	DepthStencilTextureDesc.CPUAccessFlags = 0;
	DepthStencilTextureDesc.MiscFlags = RenderTargetInfo.bCube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
	DepthStencilTexture = std::make_shared<MTexture>();
	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateTexture2D(&DepthStencilTextureDesc, nullptr, &DepthStencilTexture->GetTextureResource()));

	// DepthStencil µª½º ½ºÅÙ½Ç ºä
	D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
	DepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	if (bSingleTexture)
	{
		DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		DepthStencilViewDesc.Texture2D.MipSlice = 0;
	}
	else if (RenderTargetInfo.bCube)
	{
		DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		DepthStencilViewDesc.Texture2DArray.ArraySize = 6;
		DepthStencilViewDesc.Texture2DArray.FirstArraySlice = 0;
		DepthStencilViewDesc.Texture2DArray.MipSlice = 0;
	}
	else
	{
		DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		DepthStencilViewDesc.Texture2DArray.ArraySize = RenderTargetInfo.TextrueNum;
		DepthStencilViewDesc.Texture2DArray.FirstArraySlice = 0;
		DepthStencilViewDesc.Texture2DArray.MipSlice = 0;
	}
	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateDepthStencilView(DepthStencilTexture->GetTextureResource(), &DepthStencilViewDesc, &_pDepthStencilView));
}

ID3D11RenderTargetView* RenderTarget::AsRenderTargetView()
{
	return _pRenderTargetView;
}

ID3D11DepthStencilView* RenderTarget::getDepthStencilView()
{
	return _pDepthStencilView;
}

const FRenderTagetInfo FRenderTagetInfo::GetDefault()
{
	FRenderTagetInfo RenderTargetInfo = {};
	RenderTargetInfo.bCube = false;
	RenderTargetInfo.TextrueNum = 1;
	RenderTargetInfo.Width = g_pSetting->getResolutionWidth<UINT>();
	RenderTargetInfo.Height = g_pSetting->getResolutionHeight<UINT>();

	return RenderTargetInfo;
}

const FRenderTagetInfo FRenderTagetInfo::GetCube()
{
	FRenderTagetInfo RenderTargetInfo;
	RenderTargetInfo.bCube = true;
	RenderTargetInfo.TextrueNum = 6;
	RenderTargetInfo.Width = 2048;	// ½¦µµ¿ì¸Ê ÇØ»óµµ
	RenderTargetInfo.Height = 2048;

	return RenderTargetInfo;
}
