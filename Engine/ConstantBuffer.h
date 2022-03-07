#pragma once
#ifndef __CONSTANT_BUFFER_H__

class ConstantBuffer
{
public:
	explicit ConstantBuffer(const int bufferSize, const void *buffer);
	~ConstantBuffer();

public:
	void Update(const void *pData, const size_t dataSize);

public:
	ID3D11Buffer *const getBuffer();
private:
	ID3D11Buffer *m_pBuffer;

	//----------------------------------------------------------------------
public:
	struct BasicConstantBuffer
	{
		Mat4 transform;
	};
	struct ColorConstantBuffer
	{
		Mat4 transform;
	};
};

#define __CONSTANT_BUFFER_H__
#endif