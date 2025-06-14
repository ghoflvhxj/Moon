#include "Include.h"
#include "ConstantBuffer.h"
#include "GraphicDevice.h"

MConstantBuffer::MConstantBuffer(const uint32 bufferSize, const void *buffer, const uint32 countOfVariables)
	: _pBuffer{ nullptr }
	, Buffer{ nullptr }
	, BufferSize{ bufferSize }
	, _countOfVarialbes{ countOfVariables }
{
	D3D11_BUFFER_DESC bd		= {};
	bd.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
	bd.Usage					= D3D11_USAGE_DYNAMIC;
	bd.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags				= 0u;
	bd.ByteWidth				= BufferSize;
	bd.StructureByteStride		= 0u;

	D3D11_SUBRESOURCE_DATA sd	= {};
	sd.pSysMem					= buffer;

	Buffer = (Byte*)_aligned_malloc(bufferSize, 16);
	ZeroMemory(Buffer, bufferSize);

	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateBuffer(&bd, &sd, &_pBuffer));
}

MConstantBuffer::~MConstantBuffer()
{
	_aligned_free(Buffer);
	Buffer = nullptr;

	SafeRelease(_pBuffer);
}

void MConstantBuffer::Commit()
{
	D3D11_MAPPED_SUBRESOURCE mappedSubResource = {};
	g_pGraphicDevice->getContext()->Map(_pBuffer, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubResource);
	memcpy(mappedSubResource.pData, Buffer, BufferSize);
	g_pGraphicDevice->getContext()->Unmap(_pBuffer, 0u);
}

void MConstantBuffer::Update(const void * pData)
{
	memcpy(Buffer, pData, BufferSize);

	Commit();
}

void MConstantBuffer::SetData(int32 Offset, Byte* InData, uint32 InSize)
{
	memcpy(Buffer + Offset, InData, InSize);
}

const uint32 MConstantBuffer::getSize() const
{
	return BufferSize;
}

ID3D11Buffer *const MConstantBuffer::getRaw()
{
	return _pBuffer;
}

const uint32 MConstantBuffer::getCountOfVariables() const
{
	return _countOfVarialbes;
}