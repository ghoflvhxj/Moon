#include "Include.h"
#include "GeometryShader.h"

#include "GraphicDevice.h"

#include "ConstantBuffer.h"

MGeometryShader::MGeometryShader(const std::wstring &filePathName)
	: MShader(filePathName)
	, _pGeometryShader{ nullptr }
{
	ID3D11Device *pDevice	= g_pGraphicDevice->getDevice();
	ID3D10Blob *pBlob		= getBlob();

	FAILED_CHECK_THROW(pDevice->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &_pGeometryShader));
}

MGeometryShader::MGeometryShader()
	: MShader(TEXT(""))
	, _pGeometryShader{ nullptr }
{
	ID3D11Device *pDevice = g_pGraphicDevice->getDevice();
	ID3D10Blob *pBlob = getBlob();
}

MGeometryShader::~MGeometryShader()
{
	SafeRelease(_pGeometryShader);
}

void MGeometryShader::SetToDevice()
{
	g_pGraphicDevice->getContext()->GSSetShader(_pGeometryShader, nullptr, 0);

	std::vector<ID3D11Buffer*>& RawBuffers = GetBuffers();
	g_pGraphicDevice->getContext()->GSSetConstantBuffers(0u, CastValue<UINT>(RawBuffers.size()), RawBuffers.data());
}

ID3D11GeometryShader* MGeometryShader::getRaw()
{
	return _pGeometryShader;
}
