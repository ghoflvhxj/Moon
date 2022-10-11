#include "stdafx.h"
#include "GeometryShader.h"

#include "GraphicDevice.h"

#include "ConstantBuffer.h"

GeometryShader::GeometryShader(const std::wstring &filePathName)
	: Shader(filePathName)
	, _pGeometryShader{ nullptr }
{
	ID3D11Device *pDevice	= g_pGraphicDevice->getDevice();
	ID3D10Blob *pBlob		= getBlob();

	FAILED_CHECK_THROW(pDevice->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &_pGeometryShader));
}

GeometryShader::GeometryShader()
	: Shader(TEXT(""))
	, _pGeometryShader{ nullptr }
{
	ID3D11Device *pDevice = g_pGraphicDevice->getDevice();
	ID3D10Blob *pBlob = getBlob();
}

GeometryShader::~GeometryShader()
{
	SafeRelease(_pGeometryShader);
}

void GeometryShader::SetToDevice()
{
	g_pGraphicDevice->getContext()->GSSetShader(_pGeometryShader, nullptr, 0);

	std::vector<ID3D11Buffer*> rawBuffers;
	rawBuffers.reserve(_constantBuffers.size());
	for (auto &constantBuffer : _constantBuffers)
	{
		rawBuffers.emplace_back(constantBuffer ? constantBuffer->getRaw() : nullptr);
	}

	g_pGraphicDevice->getContext()->GSSetConstantBuffers(0u, CastValue<UINT>(rawBuffers.size()), &rawBuffers[0]);
}

ID3D11GeometryShader* GeometryShader::getRaw()
{
	return _pGeometryShader;
}
