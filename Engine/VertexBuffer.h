#pragma once

#include "cuda.h"

#ifdef PHYSX
namespace physx
{
    class PxDeformableSurface;
}
#endif

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
	uint32 VertexNum;
    uint32 VertexSize;

public:
    void Update(void* InData);

#ifdef PHYSX_CUDA
    void UpdateUsingCUDA(physx::PxDeformableSurface* DeformableSurface, uint32 VertexNum);
    CUgraphicsResource CudaResource;
#endif
};
