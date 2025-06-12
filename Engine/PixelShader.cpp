#include "Include.h"
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
    // 이거는 구조를 수정해야 할듯. 렌더러가 이 함수 내용을 직접 호출하도록
    // ConstantBuffer 업데이트 쪽도 HandlePixelShader로 옮기기
	g_pGraphicDevice->getContext()->PSSetShader(_pPixelShader, nullptr, 0);

    std::vector<ID3D11Buffer*> rawBuffers;
    rawBuffers.reserve(ConstantBuffers.size());
    for (auto& constantBuffer : ConstantBuffers)
    {
        rawBuffers.emplace_back(constantBuffer ? constantBuffer->getRaw() : nullptr);
    }

    g_pGraphicDevice->getContext()->PSSetConstantBuffers(0u, CastValue<UINT>(rawBuffers.size()), ConstantBuffers.size() > 0 ? &rawBuffers[0] : nullptr);
}

ID3D11PixelShader* PixelShader::getRaw()
{
	return _pPixelShader;
}
