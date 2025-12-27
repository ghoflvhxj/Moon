#pragma once

#ifdef PHYSX
#include "cuda.h"
namespace physx
{
    class PxDeformableSurface;
}
#endif

class MVertexBuffer final
{
public:
    explicit MVertexBuffer(const uint32 vertexSize, const uint32 vertexCount, const void* buffer, bool bInDynamic = false);
	~MVertexBuffer();

protected:
    void CreateBuffer(const uint32 InDataSize, const void* InBuffer, bool InDynamic);

public:
	void setBufferToDevice(UINT &stride, UINT &offset);
public:
	ID3D11Buffer* getBuffer();
private: 
	ID3D11Buffer *_pBuffer = nullptr;

public:
	const uint32 getVertexNum() const;
    const uint32 GetVertexSize() const { return VertexSize; }
private:
	uint32 VertexNum = 0;
    uint32 VertexSize = 0;

public:
    void Update(void* InData);
    void Update(void* InData, uint32 InNum);

#ifdef PHYSX_CUDA
    void UpdateUsingCUDA(physx::PxDeformableSurface* DeformableSurface, uint32 VertexNum);
    CUgraphicsResource CudaResource;
#endif
};
