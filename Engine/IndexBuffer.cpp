#include "Include.h"
#include "IndexBuffer.h"
#include "GraphicDevice.h"
#include "WindowException.h"

MIndexBuffer::MIndexBuffer(const uint32 elementTypeSize, const uint32 indexCount, void *buffer)
	: _pBuffer		{ nullptr }
	, _indexCount	{ indexCount }
{
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = static_cast<UINT>(elementTypeSize * indexCount);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.StructureByteStride = 0u;

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = buffer;

	if (g_pGraphicDevice->getDevice()->CreateBuffer(&bd, &sd, &_pBuffer) == E_FAIL)
	{
#ifdef _DEBUG
		throw WINDOW_EXCEPTION(GetLastError());
#endif
	}
}

MIndexBuffer::~MIndexBuffer()
{
	SafeRelease(_pBuffer);
}

void MIndexBuffer::setBufferToDevice(const UINT &offset)
{
	g_pGraphicDevice->getContext()->IASetIndexBuffer(_pBuffer, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, offset);
}

ID3D11Buffer *const MIndexBuffer::getBuffer()
{
	return _pBuffer;
}

const uint32 MIndexBuffer::getIndexCount() const
{
	return _indexCount;
}
