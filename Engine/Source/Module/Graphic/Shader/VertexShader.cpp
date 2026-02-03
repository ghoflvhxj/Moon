#include "VertexShader.h"

#include "MoonEngine.h"
#include "GraphicDevice.h"

#include "ConstantBuffer.h"

MVertexShader::MVertexShader(const std::wstring &filePathName)
	: MShader(filePathName)
	, _pVertexShader{ nullptr }
{
    ShaderType = EShaderType::Vertex;

	ID3D11Device *pDevice	= g_pGraphicDevice->getDevice();
	ID3D10Blob *pBlob		= getBlob();

	FAILED_CHECK_THROW(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &_pVertexShader));
}

MVertexShader::MVertexShader()
	: MShader(TEXT(""))
	, _pVertexShader{ nullptr }
{
	ID3D11Device *pDevice = g_pGraphicDevice->getDevice();
	ID3D10Blob *pBlob = getBlob();
}

MVertexShader::~MVertexShader()
{
	SafeRelease(_pVertexShader);
}

void MVertexShader::SetToDevice()
{
    std::vector<ID3D11Buffer*>& RawBuffers = GetBuffers();
    g_pGraphicDevice->getContext()->VSSetConstantBuffers(EnumToIndex(EConstantBufferLayer::RenderPass), CastValue<UINT>(RawBuffers.size()), RawBuffers.data());
    
    //// StructuredBuffer 설정
    //if (StructuredBuffer.IsValid())
    //{
    //    getGraphicDevice()->VSSetSRV(StructuredBuffer);
    //}

    g_pGraphicDevice->getContext()->VSSetShader(_pVertexShader, nullptr, 0);
}

ID3D11VertexShader* MVertexShader::getRaw()
{
	return _pVertexShader;
}
