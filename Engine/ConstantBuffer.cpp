#include "Include.h"
#include "ConstantBuffer.h"
#include "GraphicDevice.h"

MConstantBuffer::MConstantBuffer(const uint32 bufferSize, const void *buffer, const uint32 countOfVariables)
	: DX_Buffer{ nullptr }
	, BufferData{ nullptr }
	, BufferSize{ bufferSize }
	, VariableNum{ countOfVariables }
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

	BufferData = (Byte*)_aligned_malloc(bufferSize, 16);
	ZeroMemory(BufferData, bufferSize);

	FAILED_CHECK_THROW(g_pGraphicDevice->getDevice()->CreateBuffer(&bd, &sd, &DX_Buffer));
}

MConstantBuffer::~MConstantBuffer()
{
	_aligned_free(BufferData);
	BufferData = nullptr;

	SafeRelease(DX_Buffer);
}

void MConstantBuffer::Commit()
{
	D3D11_MAPPED_SUBRESOURCE mappedSubResource = {};
	g_pGraphicDevice->getContext()->Map(DX_Buffer, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubResource);
	memcpy(mappedSubResource.pData, BufferData, BufferSize);
	g_pGraphicDevice->getContext()->Unmap(DX_Buffer, 0u);
}

void MConstantBuffer::SetAllData(const void * pData)
{
	memcpy(BufferData, pData, BufferSize);

	Commit();
}

void MConstantBuffer::SetData(int32 Offset, const void* InData, uint32 InSize)
{
	memcpy(BufferData + Offset, InData, InSize);
}

void MConstantBuffer::SetData(const std::wstring& InName, const void* InData)
{
    auto& Iter = VariableInfos.find(InName);
    if (Iter == VariableInfos.end())
    {
        return;
    }

    const FBufferVariableInfo& VariableInfo = Iter->second;
    const FBufferVariable& Variable = Variables[VariableInfo.Index];

    // TODO 벡터가 사이즈가 다르다면 크래시가 발생함. Variable과 Vector중에 사이즈가 작은 것 선택
    SetData(Variable.Offset, InData, Variable.Size);
}

const uint32 MConstantBuffer::getSize() const
{
	return BufferSize;
}

ID3D11Buffer *const MConstantBuffer::getRaw()
{
	return DX_Buffer;
}

const uint32 MConstantBuffer::getCountOfVariables() const
{
	return VariableNum;
}

void MConstantBuffer::AddVariable(const std::wstring& InName, const FBufferVariableInfo& InVariableInfo, const FBufferVariable& InVariable)
{
    VariableInfos[InName] = InVariableInfo;
    Variables.push_back(InVariable);
}
