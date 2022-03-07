#pragma once
#ifndef __CONSTANT_BUFFER_H__

class ConstantBuffer
{
public:
	explicit ConstantBuffer(const int bufferSize, const void *buffer);
	~ConstantBuffer();

public:
	void update(const void *pData, const size_t dataSize);
	//void setBufferToDevice(UINT &stride, UINT &offset);

public:
	ID3D11Buffer *const getBuffer();
private:
	ID3D11Buffer *m_pBuffer;

};

#define __CONSTANT_BUFFER_H__
#endif