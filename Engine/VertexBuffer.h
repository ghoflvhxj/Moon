#pragma once
#ifndef __VERTEX_BUFFER_H__

class MVertexBuffer final
{
public:
	explicit MVertexBuffer(const uint32 vertexSize, const uint32 vertexCount, const void *buffer);
	~MVertexBuffer();

public:
	void setBufferToDevice(UINT &stride, UINT &offset);
public:
	ID3D11Buffer* getBuffer();
private: 
	ID3D11Buffer *_pBuffer;

public:
	const uint32 getVertexCount() const;
private:
	uint32 _vertexCount;
};

#define __VERTEX_BUFFER_H__
#endif