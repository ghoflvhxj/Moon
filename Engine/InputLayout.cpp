#include "stdafx.h"
#include "InputLayout.h"

#include "GraphicDevice.h"

InputLayout::InputLayout(D3D11_INPUT_ELEMENT_DESC desc[], const uint32 elementNum, const wchar_t *vertexShader, const wchar_t *pixelShader)
{
	//ID3D11Device		*pDevice	= g_pGraphicDevice->getDevice();
	//ID3D11DeviceContext *pContext	= g_pGraphicDevice->getContext();

	//ID3DBlob *pBlob = nullptr;
	//FAILED_CHECK_THROW(D3DReadFileToBlob(vertexShader, &pBlob));
	//FAILED_CHECK_THROW(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &_pPixelShader));
	////pContext->PSSetShader(_pPixelShader, nullptr, 0);

	//pBlob->Release();

	//FAILED_CHECK_THROW(D3DReadFileToBlob(pixelShader, &pBlob));
	//FAILED_CHECK_THROW(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &_pVertexShader));
	////pContext->VSSetShader(_pVertexShader, nullptr, 0);

	//FAILED_CHECK_THROW(pDevice->CreateInputLayout(desc, elementNum, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &_pInputLayout));
	//pBlob->Release();
}
