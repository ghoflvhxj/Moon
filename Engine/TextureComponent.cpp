#include "Include.h"
#include "TextureComponent.h"

#include "GraphicDevice.h"

#include "DirectXTK/WICTextureLoader.h"

using namespace DirectX;

MTexture::MTexture(const wchar_t *fileName)
	: _rawTexture{ nullptr }
	, _pResourceView{ nullptr }
{
	loadTextureFile(fileName);
}

MTexture::MTexture(const char *fileName)
	: _rawTexture{ nullptr }
	, _pResourceView{ nullptr }
{
	wchar_t wFileName[MAX_PATH] = {};

	int len = MultiByteToWideChar(CP_ACP, 0, fileName, static_cast<int>(strlen(fileName)), NULL, NULL);
	MultiByteToWideChar(CP_ACP, 0, fileName, static_cast<int>(strlen(fileName)), wFileName, len);

	loadTextureFile(wFileName);
}

MTexture::MTexture(MTexture::RawTexturePtr pTexture)
	: _rawTexture{ pTexture }
	, _pResourceView{ nullptr }
{
	D3D11_TEXTURE2D_DESC texturDesc = {};
	pTexture->GetDesc(&texturDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = texturDesc.Format;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipLevels = 1;

	g_pGraphicDevice->getDevice()->CreateShaderResourceView(_rawTexture, &desc, &_pResourceView);
}

MTexture::MTexture(ID3D11ShaderResourceView* pShaderResourceView)
	: _rawTexture{ nullptr }
	, _pResourceView{ pShaderResourceView }
{
	SAFE_ADDREF(_pResourceView);
}

MTexture::MTexture()
	: _rawTexture{ nullptr }
	, _pResourceView{ nullptr }
{

}

MTexture::~MTexture()
{
	SAFE_RELEASE(_pResourceView);
	SAFE_RELEASE(_rawTexture);
}

const bool MTexture::loadTextureFile(const wchar_t *fileName)
{
	if(nullptr != _pResourceView)
		_pResourceView->Release();
	if (nullptr != _rawTexture)
		_rawTexture->Release();


	FAILED_CHECK_THROW(CreateWICTextureFromFile(g_pGraphicDevice->getDevice(), fileName, (ID3D11Resource **)&_rawTexture, &_pResourceView));
	
	return true;
}

void MTexture::setTexture(const uint32 index)
{
	g_pGraphicDevice->getContext()->PSSetShaderResources(index, 1, &_pResourceView);
}

MTexture::RawTexturePtr& MTexture::getRawTexturePointer()
{
	return _rawTexture;
}

MTexture::ResourceViewPtr& MTexture::getRawResourceViewPointer()
{
	return _pResourceView;
}
