#pragma once

#include "Include.h"

class MBuffer
{
public:
    MBuffer() = default;
    virtual ~MBuffer() = default;

public:
    virtual void Init(const uint32 InElementSize, const uint32 InElementNum);
public:
    uint32 GetElementSize() const { return ElementSize; }
    uint32 GetElementNum() const { return ElementNum; }
    uint32 GetBufferSize() const { return BufferSize; }
protected:
    uint32 ElementSize = 0;
    uint32 ElementNum = 0;
    uint32 BufferSize = 0;

public:
    void SetBufferID(uint32 InBufferID) { BufferID = InBufferID; }
    uint32 GetBufferID() const { return BufferID; }
protected:
    uint32 BufferID = 0;
};

// 다른 버퍼와는 다르게 바인딩을 위해서 SRV를 준비해야 함.
// VertexBuffer     - IASetVertexBuffer
// IndexBuffer      - IASetIndexBuffer 
// ConstantBuffer   - XXSetConstantBuffers
// StructuredBuffer - XXSetShaderResourceView

class MStructuredBuffer : public MBuffer
{
public:
    MStructuredBuffer() = default;

public:
    void SetSRVID(uint32 InSRVID) { SRVID = InSRVID; }
    uint32 GetSRVID() const { return SRVID; }
protected:
    uint32 SRVID = 0;
};