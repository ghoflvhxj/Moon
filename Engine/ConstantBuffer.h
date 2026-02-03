#pragma once

#include "Include.h"

class MMaterial;
struct FBufferVariableInfo;
struct FBufferVariable;

class MConstantBuffer
{
public:
	explicit MConstantBuffer(const uint32 size, const void *buffer, const uint32 countOfVariables);
	~MConstantBuffer();

    /*************************
        CPU -> GPU
    *************************/
public:
	// CPU 데이터를 GPU에 올림
	void Commit();
	// 데이터를 업데이트 하고 디바이스에 올림
	//void SetAllData(const void *pData);
    
    /*************************
        CPU 데이터 업데이트
    *************************/
public:
	// 특정 CPU 데이터만 업데이트 할 떄 사용
	void SetData(int32 Offset, const void* InData, uint32 InSize);
    void SetData(const std::wstring& InName, const void* InData);

public:
	const uint32 getSize() const;
private:
	uint32 BufferSize;

public:
	ID3D11Buffer *const getRaw();
private:
	ID3D11Buffer *DX_Buffer;

public:
	const uint32 getCountOfVariables() const;
private:
	uint32 VariableNum;

private:
	Byte* BufferData;

public:
    void AddVariable(const std::wstring& InName, const FBufferVariableInfo& InVariableInfo, const FBufferVariable& InVariable);
protected:
    // 이름, 버퍼 변수 쌍을 저장함
    std::map<std::wstring, FBufferVariableInfo> VariableInfos;
    // 변수의 데이터를 저장함
    std::vector<FBufferVariable> Variables;
};