#include "Include.h"
#include "VertexBuffer.h"
#include "GraphicDevice.h"
#include "WindowException.h"

VertexBuffer::VertexBuffer(const uint32 vertexSize, const uint32 vertexCount, const void *buffer)
	: _pBuffer		{ nullptr }
	, _vertexCount	{ vertexCount }
{
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth			= static_cast<UINT>(vertexSize * vertexCount);
	bd.Usage				= D3D11_USAGE_DEFAULT;
	bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags		= 0u;
	bd.MiscFlags			= 0u;
	bd.StructureByteStride	= 0u;
	
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = buffer;

	if (g_pGraphicDevice->getDevice()->CreateBuffer(&bd, &sd, &_pBuffer) == E_FAIL)
	{
#ifdef _DEBUG
		throw WINDOW_EXCEPTION(GetLastError());
#endif
	}
}

VertexBuffer::~VertexBuffer()
{
	SafeRelease(_pBuffer);
}

void VertexBuffer::setBufferToDevice(UINT &stride, UINT &offset)
{
	g_pGraphicDevice->getContext()->IASetVertexBuffers(0, 1, &_pBuffer, &stride, &offset);
}

ID3D11Buffer* VertexBuffer::getBuffer()
{
	return _pBuffer;
}

const uint32 VertexBuffer::getVertexCount() const
{
	return _vertexCount;
}
