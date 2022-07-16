#include "stdafx.h"
#include "ConstantBuffer.h"
#include "GraphicDevice.h"

ConstantBuffer::ConstantBuffer(const uint32 bufferSize, const void *buffer, const uint32 countOfVariables)
	: _pBuffer{ nullptr }
	, _size{ bufferSize }
	, _countOfVarialbes{ countOfVariables }
{
	D3D11_BUFFER_DESC bd		= {};
	bd.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
	bd.Usage					= D3D11_USAGE_DYNAMIC;
	bd.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags				= 0u;
	bd.ByteWidth				= _size;
	bd.StructureByteStride		= 0u;

	D3D11_SUBRESOURCE_DATA sd	= {};
	sd.pSysMem					= buffer;

	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateBuffer(&bd, &sd, &_pBuffer));
}

ConstantBuffer::~ConstantBuffer()
{
	SafeRelease(_pBuffer);
}

void ConstantBuffer::update(const void * pData, const size_t dataSize)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubResource = {};
	g_pGraphicDevice->getContext()->Map(_pBuffer, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubResource);
	memcpy(mappedSubResource.pData, pData, dataSize);
	g_pGraphicDevice->getContext()->Unmap(_pBuffer, 0u);
}

//void ConstantBuffer::setBufferToDevice(UINT &stride, UINT &offset)
//{
//
//}

const uint32 ConstantBuffer::getSize() const
{
	return _size;
}

ID3D11Buffer *const ConstantBuffer::getRaw()
{
	return _pBuffer;
}

const uint32 ConstantBuffer::getCountOfVariables() const
{
	return _countOfVarialbes;
}