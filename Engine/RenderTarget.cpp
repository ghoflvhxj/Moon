#include "Include.h"
#include "RenderTarget.h"

#include "GraphicDevice.h"
#include "Material.h"

#include "MeshComponent.h"
#include "TextureComponent.h"

#include "MainGame.h"
#include "MainGameSetting.h"

RenderTarget::RenderTarget()
	: _pRenderTargetTexture{ nullptr }
	, _pRenderTargetView{ nullptr }
	, _pDepthStencilTexture{ nullptr }
	, _pDepthStencilView{ nullptr }
{
	RenderTagetInfo info = {};
	info.textureArrayCount = 1;
	info.width = g_pSetting->getResolutionWidth<UINT>();
	info.height = g_pSetting->getResolutionHeight<UINT>();

	initializeTexture(info);
}

RenderTarget::RenderTarget(RenderTagetInfo &info)
	: _pRenderTargetTexture{ nullptr }
	, _pRenderTargetView{ nullptr }
	, _pDepthStencilTexture{ nullptr }
	, _pDepthStencilView{ nullptr }
{
	initializeTexture(info);
}

RenderTarget::~RenderTarget()
{
	SAFE_RELEASE(_pRenderTargetView)
	SAFE_RELEASE(_pDepthStencilView);
}

std::shared_ptr<MTexture> RenderTarget::AsTexture()
{
	return _pRenderTargetTexture;
}

void RenderTarget::initializeTexture(RenderTagetInfo &renderTargetInfo)
{
	// RenderTargetView
	D3D11_TEXTURE2D_DESC renderTagetDesc = {};
	renderTagetDesc.Width = renderTargetInfo.width;
	renderTagetDesc.Height = renderTargetInfo.height;
	renderTagetDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderTagetDesc.SampleDesc.Count = 1;
	renderTagetDesc.SampleDesc.Quality = 0;
	renderTagetDesc.ArraySize = renderTargetInfo.textureArrayCount;
	renderTagetDesc.MipLevels = 1;
	renderTagetDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTagetDesc.Format = renderTargetInfo.textureArrayCount == 1 ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R32_TYPELESS;
	renderTagetDesc.CPUAccessFlags = 0;
	renderTagetDesc.MiscFlags = 0;

	_pRenderTargetTexture = std::make_shared<MTexture>();
	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateTexture2D(&renderTagetDesc, nullptr, &_pRenderTargetTexture->getRawTexturePointer()));

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = { };
	renderTargetViewDesc.Format = renderTargetInfo.textureArrayCount == 1 ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R32_FLOAT;
	if (renderTargetInfo.textureArrayCount <= 1)
	{
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
	}
	else
	{
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		renderTargetViewDesc.Texture2DArray.MipSlice = 0;
		renderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;
		renderTargetViewDesc.Texture2DArray.ArraySize = renderTargetInfo.textureArrayCount;
	}
	
	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateRenderTargetView(_pRenderTargetTexture->getRawTexturePointer(), &renderTargetViewDesc, &_pRenderTargetView));

	// ShaderResouceView
	D3D11_SHADER_RESOURCE_VIEW_DESC svd = { };
	svd.Format = renderTargetInfo.textureArrayCount == 1 ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R32_FLOAT;
	if (renderTargetInfo.textureArrayCount <= 1)
	{
		svd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		svd.Texture2D.MipLevels = -1;
		svd.Texture2D.MostDetailedMip = 0;
	}
	else
	{
		svd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		svd.Texture2DArray.MipLevels = -1;
		svd.Texture2DArray.MostDetailedMip = 0;
		svd.Texture2DArray.ArraySize = renderTargetInfo.textureArrayCount;
		svd.Texture2DArray.FirstArraySlice = 0;
	}

	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateShaderResourceView(_pRenderTargetTexture->getRawTexturePointer(), &svd, &_pRenderTargetTexture->getRawResourceViewPointer()));

	// DepthStencilView
	D3D11_TEXTURE2D_DESC depthStencilDesc = { };
	depthStencilDesc.Width = renderTargetInfo.width;
	depthStencilDesc.Height = renderTargetInfo.height;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.ArraySize = renderTargetInfo.textureArrayCount;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	_pDepthStencilTexture = std::make_shared<MTexture>();
	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateTexture2D(&depthStencilDesc, nullptr, &_pDepthStencilTexture->getRawTexturePointer()));
	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateDepthStencilView(_pDepthStencilTexture->getRawTexturePointer(), nullptr, &_pDepthStencilView));
}

ID3D11RenderTargetView* RenderTarget::AsRenderTargetView()
{
	return _pRenderTargetView;
}

ID3D11DepthStencilView* RenderTarget::getDepthStencilView()
{
	return _pDepthStencilView;
}