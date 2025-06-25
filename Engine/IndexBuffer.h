#pragma once
#ifndef __INDEX_BUFFER_H__

class MIndexBuffer
{
public:
	explicit MIndexBuffer(const uint32 elementTypeSize, const uint32 indexCount, void *buffer);
	~MIndexBuffer();

public:
	void setBufferToDevice(const UINT &offset);
public:
	ID3D11Buffer *const getBuffer();
private:
	ID3D11Buffer *_pBuffer;

public:
	const uint32 getIndexCount() const;
private:
	uint32 _indexCount;
};

#define __INDEX_BUFFER_H__
#endif