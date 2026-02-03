#pragma once

#include "Include.h"

/****************************************
    Vertex, Index   ->  메시를 이용해 생성됨. 디바이스가 관리.
    Constant        ->  쉐이더가 관리
    Structured      ->  오브젝트를 이용해 생성됨. 디바이스가 관리.
****************************************/

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
    bool IsValid() const { return Slot != -1; }

public:
    void SetSlot(const uint32 InSlot) { Slot = static_cast<int32>(InSlot); }
    int32 GetSlot() const { return Slot; }
protected:
    int32 Slot = -1;

public:
    void SetBufferID(uint32 InBufferID) { BufferID = static_cast<int32>(InBufferID); }
    int32 GetBufferID() const { return BufferID; }
protected:
    int32 BufferID = -1;
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
    void SetSRVID(uint32 InSRVID) { SRVID = static_cast<int32>(InSRVID); }
    int32 GetSRVID() const { return SRVID; }
protected:
    int32 SRVID = -1;

public:
    void SetUAVID(uint32 InUAVID) { UAVID = static_cast<int32>(InUAVID); }
    int32 GetUAVID() const { return UAVID; }
protected:
    int32 UAVID = -1;
};