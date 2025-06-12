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
    // 이거는 구조를 수정해야 할듯. 렌더러가 이 함수 내용을 직접 호출하도록
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
