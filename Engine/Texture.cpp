#include "Texture.h"

#include "DirectXTK/WICTextureLoader.h"
#include "GraphicDevice.h"
#include "Core/ResourceLoader.h"
#include "Core/ResourceManager.h"
#include "Utility/PerformanceTimer.h"

using namespace DirectX;

MTexture::MTexture(const std::wstring& InPath)
{
	loadTextureFile(InPath.c_str());
}

MTexture::MTexture(ID3D11Texture2D* pTexture)
	: Texture(pTexture)
{
	D3D11_TEXTURE2D_DESC texturDesc = {};
	pTexture->GetDesc(&texturDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = texturDesc.Format;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipLevels = 1;

	g_pGraphicDevice->getDevice()->CreateShaderResourceView(Texture, &desc, &ShaderResurceView);
}

MTexture::MTexture(ID3D11ShaderResourceView* pShaderResourceView)
{
}

MTexture::MTexture()
{

}

MTexture::MTexture(const MTexture& Rhs)
{
    Path = Rhs.Path;
}

MTexture::~MTexture()
{
	OutputDebugStringW((TEXT("Release Texture: ") + GetAssetPath() + TEXT("\n")).c_str());
    SafeRelease(ShaderResurceView);
    SafeRelease(Texture);
}

bool MTexture::Load(const std::wstring& InPath)
{
    if (Super::Load(InPath))
    {
        loadTextureFile(Path.c_str());
        return true;
    }

    return false;
}

const bool MTexture::loadTextureFile(const wchar_t *fileName)
{
    SetAssetPath(fileName);

    SafeRelease(ShaderResurceView);
    SafeRelease(Texture);

#ifdef _DEBUG
    std::wstring Msg = GetAssetPath() + TEXT("텍스쳐 로딩 시간: ");
    PerformanceTimer Pt(Msg);
#endif

    ID3D11ShaderResourceView* NewSRV = nullptr;
	FAILED_CHECK_THROW(CreateWICTextureFromFile(g_pGraphicDevice->getDevice(), fileName, (ID3D11Resource**)&Texture, &ShaderResurceView));
	
	return true;
}

//void MTexture::setTexture(const uint32 index)
//{
//	g_pGraphicDevice->getContext()->PSSetShaderResources(index, 1, &_pResourceView);
//}

void MTexture::SetTexture(ID3D11Texture2D* InTexture)
{
    Texture = InTexture;
}

ID3D11Texture2D* MTexture::GetTexture()
{
	return Texture;
}

void MTexture::SetShaderResourceView(ID3D11ShaderResourceView* InSRV)
{
    ShaderResurceView = InSRV;
}

ID3D11ShaderResourceView* MTexture::GetShaderResourceView()
{
	return ShaderResurceView;
}

const bool MTexture::GetResolution(uint32& OutWidth, uint32& OutHeight)
{
	if (Texture == nullptr)
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC TextureDesc = {};
    Texture->GetDesc(&TextureDesc);
	
	OutWidth = TextureDesc.Width;
	OutHeight = TextureDesc.Height;

	return true;
}
