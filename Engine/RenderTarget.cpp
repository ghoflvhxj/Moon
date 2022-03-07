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
	, _pShaderResouceView{ nullptr }
	, _pDepthStencilTexture{ nullptr }
	, _pDepthStencilView{ nullptr }
{
	initializeTexture();
	initializeMesh();
}

RenderTarget::~RenderTarget()
{
	SAFE_RELEASE(_pRenderTargetView);
	SAFE_RELEASE(_pRenderTargetTexture);
}

void RenderTarget::Update(const Time deltaTime)
{
	_pMeshComponent->SceneComponent::Update(deltaTime);
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

	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateTexture2D(&renderTagetDesc, nullptr, &_pRenderTargetTexture));

	D3D11_RENDER_TARGET_VIEW_DESC rvd = { };
	rvd.Format = renderTagetDesc.Format;
	rvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rvd.Texture2D.MipSlice = 0;
	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateRenderTargetView(_pRenderTargetTexture, &rvd, &_pRenderTargetView));

	D3D11_SHADER_RESOURCE_VIEW_DESC svd = { };
	svd.Format = renderTagetDesc.Format;
	svd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	svd.Texture2D.MipLevels = -1;
	svd.Texture2D.MostDetailedMip = 0;
	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateShaderResourceView(_pRenderTargetTexture, &svd, &_pShaderResouceView));

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

	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateTexture2D(&depthStencilDesc, nullptr, &_pDepthStencilTexture));
	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateDepthStencilView(_pDepthStencilTexture, nullptr, &_pDepthStencilView));
}

void RenderTarget::initializeMesh()
{
	_pMeshComponent = std::make_shared<MeshComponent>();	// BoxComponent
	_pMeshComponent->setScale(Vec3{ 200.f ,200.f, 1.f });
	_pMeshComponent->setTranslation(Vec3{ 0.f, 0.f, 1.f });
	_pMeshComponent->setRenderMode(MeshComponent::RenderMode::Orthogonal);

	std::shared_ptr<TextureComponent> _pTextureComponent = std::make_shared<TextureComponent>(_pRenderTargetTexture);
	_pMeshComponent->setTexture(TextureType::Diffuse, _pTextureComponent);
}

std::shared_ptr<MeshComponent> RenderTarget::getMeshComponent()
{
	return _pMeshComponent;
}

ID3D11RenderTargetView* RenderTarget::getRenderTargetView()
{
	return _pRenderTargetView;
}

ID3D11ShaderResourceView* RenderTarget::getShaderResouceView()
{
	return _pShaderResouceView;
}

ID3D11DepthStencilView* RenderTarget::getDepthStencilView()
{
	return _pDepthStencilView;
}

void RenderTarget::Render()
{
	_pMeshComponent->render();
}
