﻿#include "Include.h"
#include "PixelShader.h"

#include "GraphicDevice.h"

#include "ConstantBuffer.h"

PixelShader::PixelShader(const std::wstring &filePathName)
	: MShader(filePathName)
	, _pPixelShader{ nullptr }
{
	ID3D11Device *pDevice	= g_pGraphicDevice->getDevice();
	ID3D10Blob *pBlob		= getBlob();

	FAILED_CHECK_THROW(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &_pPixelShader));
}

PixelShader::PixelShader()
	: MShader(TEXT(""))
	, _pPixelShader{ nullptr }
{
	ID3D11Device *pDevice = g_pGraphicDevice->getDevice();
	ID3D10Blob *pBlob = getBlob();
}

PixelShader::~PixelShader()
{
	SafeRelease(_pPixelShader);
}

void PixelShader::SetToDevice()
{
	g_pGraphicDevice->getContext()->PSSetShader(_pPixelShader, nullptr, 0);

    std::vector<ID3D11Buffer*>& RawBuffers = GetBuffers();
    g_pGraphicDevice->getContext()->PSSetConstantBuffers(0u, CastValue<UINT>(RawBuffers.size()), RawBuffers.data());
}

ID3D11PixelShader* PixelShader::getRaw()
{
	return _pPixelShader;
}
