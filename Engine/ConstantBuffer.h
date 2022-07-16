#pragma once
#ifndef __CONSTANT_BUFFER_H__

class Material;

class ConstantBuffer
{
public:
	explicit ConstantBuffer(const uint32 size, const void *buffer, const uint32 countOfVariables);
	~ConstantBuffer();

public:
	void update(const void *pData, const size_t dataSize);
	//void setBufferToDevice(UINT &stride, UINT &offset);

public:
	const uint32 getSize() const;
private:
	uint32 _size;

public:
	ID3D11Buffer *const getRaw();
private:
	ID3D11Buffer *_pBuffer;

public:
	const uint32 getCountOfVariables() const;
private:
	uint32 _countOfVarialbes;

};

#define __CONSTANT_BUFFER_H__
#endif