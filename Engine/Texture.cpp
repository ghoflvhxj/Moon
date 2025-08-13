#include "Texture.h"

#include "DirectXTK/WICTextureLoader.h"
#include "GraphicDevice.h"
#include "Core/ResourceLoader.h"
#include "Core/ResourceManager.h"

using namespace DirectX;

MTexture::MTexture(const std::wstring& InPath)
	: MAsset(InPath)
    , _rawTexture{ nullptr }
	, _pResourceView{ nullptr }
{
	loadTextureFile(InPath.c_str());
}

MTexture::MTexture(ID3D11Texture2D* pTexture)
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

MTexture::MTexture(const MTexture& Rhs)
    : _rawTexture{ nullptr }
    , _pResourceView{ nullptr }
{
    Path = Rhs.Path;
}

MTexture::~MTexture()
{
	OutputDebugStringW(TEXT("Release Texture\n"));
	SafeRelease(_pResourceView);
	SafeRelease(_rawTexture);
}

bool MTexture::Load()
{
    if (Super::Load())
    {
        loadTextureFile(Path.c_str());
        return true;
    }

    return false;
}

const bool MTexture::loadTextureFile(const wchar_t *fileName)
{
	if(nullptr != _pResourceView)
		SafeRelease(_pResourceView);
	if (nullptr != _rawTexture)
		SafeRelease(_rawTexture);

	FAILED_CHECK_THROW(CreateWICTextureFromFile(g_pGraphicDevice->getDevice(), fileName, (ID3D11Resource **)&_rawTexture, &_pResourceView));
	
	return true;
}

void MTexture::setTexture(const uint32 index)
{
	g_pGraphicDevice->getContext()->PSSetShaderResources(index, 1, &_pResourceView);
}

ID3D11Texture2D*& MTexture::GetTextureResource()
{
	return _rawTexture;
}

ID3D11ShaderResourceView*& MTexture::getRawResourceViewPointer()
{
	return _pResourceView;
}

const bool MTexture::GetResolution(uint32& OutWidth, uint32& OutHeight)
{
	if (_rawTexture == nullptr)
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC TextureDesc = {};
	_rawTexture->GetDesc(&TextureDesc);
	
	OutWidth = TextureDesc.Width;
	OutHeight = TextureDesc.Height;

	return true;
}
