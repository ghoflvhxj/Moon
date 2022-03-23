#pragma once
#ifndef __VERTEX_BUFFER_H__

class VertexBuffer final
{
public:
	explicit VertexBuffer(const uint32 vertexSize, const uint32 vertexCount, void *buffer);
	~VertexBuffer();

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