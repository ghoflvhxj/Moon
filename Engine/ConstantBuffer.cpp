#include "stdafx.h"
#include "ConstantBuffer.h"
#include "GraphicDevice.h"

ConstantBuffer::ConstantBuffer(const int bufferSize, const void *buffer)
	: m_pBuffer{ nullptr }
{
	D3D11_BUFFER_DESC bd		= {};
	bd.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
	bd.Usage					= D3D11_USAGE_DYNAMIC;
	bd.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags				= 0u;
	bd.ByteWidth				= bufferSize;
	bd.StructureByteStride		= 0u;

	D3D11_SUBRESOURCE_DATA sd	= {};
	sd.pSysMem					= buffer;

	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateBuffer(&bd, &sd, &m_pBuffer));
}

ConstantBuffer::~ConstantBuffer()
{
	m_pBuffer->Release();
}

void ConstantBuffer::update(const void * pData, const size_t dataSize)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubResource = {};
	g_pGraphicDevice->getContext()->Map(m_pBuffer, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubResource);
	memcpy(mappedSubResource.pData, pData, dataSize);
	g_pGraphicDevice->getContext()->Unmap(m_pBuffer, 0u);
}

ID3D11Buffer *const ConstantBuffer::getBuffer()
{
	return m_pBuffer;
}
