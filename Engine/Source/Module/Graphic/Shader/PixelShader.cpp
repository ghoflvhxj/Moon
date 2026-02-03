#include "Include.h"
#include "Module/Graphic/Shader/PixelShader.h"

#include "GraphicDevice.h"

#include "ConstantBuffer.h"

MPixelShader::MPixelShader(const std::wstring &filePathName)
	: MShader(filePathName)
	, _pPixelShader{ nullptr }
{
    ShaderType = EShaderType::Pixel;

	ID3D11Device *pDevice	= g_pGraphicDevice->getDevice();
	ID3D10Blob *pBlob		= getBlob();

	FAILED_CHECK_THROW(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &_pPixelShader));
}

MPixelShader::MPixelShader()
	: MShader(TEXT(""))
	, _pPixelShader{ nullptr }
{
	ID3D11Device *pDevice = g_pGraphicDevice->getDevice();
	ID3D10Blob *pBlob = getBlob();
}

MPixelShader::~MPixelShader()
{
	SafeRelease(_pPixelShader);
}

void MPixelShader::SetToDevice()
{
	g_pGraphicDevice->getContext()->PSSetShader(_pPixelShader, nullptr, 0);

    std::vector<ID3D11Buffer*>& RawBuffers = GetBuffers();
    g_pGraphicDevice->getContext()->PSSetConstantBuffers(EnumToIndex(EConstantBufferLayer::RenderPass), CastValue<UINT>(RawBuffers.size()), RawBuffers.data());
}

ID3D11PixelShader* MPixelShader::getRaw()
{
	return _pPixelShader;
}
