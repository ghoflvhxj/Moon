#pragma once
#ifndef __CONSTANT_BUFFER_H__

class MMaterial;

class MConstantBuffer
{
public:
	explicit MConstantBuffer(const uint32 size, const void *buffer, const uint32 countOfVariables);
	~MConstantBuffer();

public:
	// 데이터를 디바이스에 올림
	void Commit();
	// 데이터를 업데이트 하고 디바이스에 올림
	void Update(const void *pData);
	// 특정 데이터만 업데이트 할 떄 사용
	void SetData(int32 Offset, Byte* InData, uint32 InSize);

	//void setBufferToDevice(UINT &stride, UINT &offset);

public:
	const uint32 getSize() const;
private:
	uint32 BufferSize;

public:
	ID3D11Buffer *const getRaw();
private:
	ID3D11Buffer *_pBuffer;

public:
	const uint32 getCountOfVariables() const;
private:
	uint32 _countOfVarialbes;

private:
	Byte* Buffer;

};

#define __CONSTANT_BUFFER_H__
#endif