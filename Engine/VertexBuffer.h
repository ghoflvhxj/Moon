#pragma once

#include "cuda.h"

namespace physx
{
    class PxDeformableSurface;
}

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
    uint32 VertexSize;

public:
    void UpdateUsingCUDA(physx::PxDeformableSurface* DeformableSurface, uint32 VertexNum);
    CUgraphicsResource CudaResource;
};
