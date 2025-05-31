#include "Include.h"
#include "VertexShader.h"

#include "GraphicDevice.h"

#include "ConstantBuffer.h"

VertexShader::VertexShader(const std::wstring &filePathName)
	: MShader(filePathName)
	, _pVertexShader{ nullptr }
{
	ID3D11Device *pDevice	= g_pGraphicDevice->getDevice();
	ID3D10Blob *pBlob		= getBlob();

	FAILED_CHECK_THROW(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &_pVertexShader));
}

VertexShader::VertexShader()
	: MShader(TEXT(""))
	, _pVertexShader{ nullptr }
{
	ID3D11Device *pDevice = g_pGraphicDevice->getDevice();
	ID3D10Blob *pBlob = getBlob();
}

VertexShader::~VertexShader()
{
	SafeRelease(_pVertexShader);
}

void VertexShader::SetToDevice()
{
	g_pGraphicDevice->getContext()->VSSetShader(_pVertexShader, nullptr, 0);

	std::vector<ID3D11Buffer*> rawBuffers;
	rawBuffers.reserve(ConstantBuffers.size());
	for (auto &constantBuffer : ConstantBuffers)
	{
		rawBuffers.emplace_back(constantBuffer ? constantBuffer->getRaw() : nullptr);
	}

	g_pGraphicDevice->getContext()->VSSetConstantBuffers(0u, CastValue<UINT>(rawBuffers.size()), &rawBuffers[0]);
}

ID3D11VertexShader* VertexShader::getRaw()
{
	return _pVertexShader;
}
