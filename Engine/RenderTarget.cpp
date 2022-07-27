#include "stdafx.h"
#include "RenderTarget.h"

#include "GraphicDevice.h"
#include "Material.h"
#include "SceneRenderer.h"

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
	initializeTexture();
}

RenderTarget::~RenderTarget()
{
	SAFE_RELEASE(_pRenderTargetView)
	SAFE_RELEASE(_pDepthStencilView);
}

std::shared_ptr<TextureComponent> RenderTarget::AsTexture()
{
	return _pRenderTargetTexture;
}

void RenderTarget::initializeTexture()
{
	// RenderTargetView, ShaderResouceView
	D3D11_TEXTURE2D_DESC renderTagetDesc = {};
	renderTagetDesc.Width = static_cast<UINT>(g_pSetting->getResolutionWidth());
	renderTagetDesc.Height = static_cast<UINT>(g_pSetting->getResolutionHeight());
	renderTagetDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderTagetDesc.SampleDesc.Count = 1;
	renderTagetDesc.SampleDesc.Quality = 0;
	renderTagetDesc.ArraySize = 1;
	renderTagetDesc.MipLevels = 1;
	renderTagetDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTagetDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTagetDesc.CPUAccessFlags = 0;
	renderTagetDesc.MiscFlags = 0;

	_pRenderTargetTexture = std::make_shared<TextureComponent>();
	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateTexture2D(&renderTagetDesc, nullptr, &_pRenderTargetTexture->getRawTexturePointer()));

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = { };
	renderTargetViewDesc.Format = renderTagetDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateRenderTargetView(_pRenderTargetTexture->getRawTexturePointer(), &renderTargetViewDesc, &_pRenderTargetView));

	D3D11_SHADER_RESOURCE_VIEW_DESC svd = { };
	svd.Format = renderTagetDesc.Format;
	svd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	svd.Texture2D.MipLevels = -1;
	svd.Texture2D.MostDetailedMip = 0;
	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateShaderResourceView(_pRenderTargetTexture->getRawTexturePointer(), &svd, &_pRenderTargetTexture->getRawResourceViewPointer()));

	// DepthStencilView
	D3D11_TEXTURE2D_DESC depthStencilDesc = { };
	depthStencilDesc.Width = static_cast<UINT>(g_pSetting->getResolutionWidth());
	depthStencilDesc.Height = static_cast<UINT>(g_pSetting->getResolutionHeight());
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	_pDepthStencilTexture = std::make_shared<TextureComponent>();
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