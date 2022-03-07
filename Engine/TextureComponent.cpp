#include "stdafx.h"
#include "TextureComponent.h"

#include "GraphicDevice.h"

#include "WICTextureLoader.h"

using namespace DirectX;

TextureComponent::TextureComponent(const wchar_t *fileName)
	: Component()
	, _pTexture{ nullptr }
	, _pResourceView{ nullptr }
{
	loadTextureFile(fileName);
}

TextureComponent::TextureComponent(const char *fileName)
	: Component()
	, _pTexture{ nullptr }
	, _pResourceView{ nullptr }
{
	wchar_t wFileName[MAX_PATH] = {};

	int len = MultiByteToWideChar(CP_ACP, 0, fileName, static_cast<int>(strlen(fileName)), NULL, NULL);
	MultiByteToWideChar(CP_ACP, 0, fileName, static_cast<int>(strlen(fileName)), wFileName, len);

	loadTextureFile(wFileName);
}

TextureComponent::TextureComponent(ID3D11Texture2D *pTexture)
	: _pTexture{ pTexture }
	, _pResourceView{ nullptr }
{
	D3D11_TEXTURE2D_DESC texturDesc = {};
	pTexture->GetDesc(&texturDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = texturDesc.Format;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipLevels = 1;

	g_pGraphicDevice->getDevice()->CreateShaderResourceView(_pTexture, &desc, &_pResourceView);
}

TextureComponent::TextureComponent(ID3D11ShaderResourceView *pShaderResourceView)
	: _pTexture{ nullptr }
	, _pResourceView{ pShaderResourceView }
{
	_pResourceView->AddRef();
}

TextureComponent::~TextureComponent()
{
	SAFE_RELEASE(_pResourceView);
	SAFE_RELEASE(_pTexture);
}

const bool TextureComponent::loadTextureFile(const wchar_t *fileName)
{
	if(nullptr != _pResourceView)
		_pResourceView->Release();
	if (nullptr != _pTexture)
		_pTexture->Release();

	FAILED_CHECK_THROW(CreateWICTextureFromFile(g_pGraphicDevice->getDevice(), fileName, (ID3D11Resource **)&_pTexture, &_pResourceView));
	
	return true;
}

void TextureComponent::setTexture(const uint32 index)
{
	g_pGraphicDevice->getContext()->PSSetShaderResources(index, 1, &_pResourceView);
}
