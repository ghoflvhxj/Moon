#include "Include.h"
#include "VertexBuffer.h"
#include "GraphicDevice.h"
#include "WindowException.h"

#include "cuda.h"
#include "cudaD3D11.h"
#include "driver_types.h"
#include "MPhysX.h"

MVertexBuffer::MVertexBuffer(const uint32 vertexSize, const uint32 vertexCount, const void *buffer)
	: _pBuffer		{ nullptr }
	, _vertexCount	{ vertexCount }
    , VertexSize     { vertexSize }
{
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth			= static_cast<UINT>(vertexSize * vertexCount);
	bd.Usage				= D3D11_USAGE_DEFAULT;
	bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags		= 0u;
	bd.MiscFlags			= 0u;
	bd.StructureByteStride	= 0u;
	
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = buffer;

	if (g_pGraphicDevice->getDevice()->CreateBuffer(&bd, &sd, &_pBuffer) == E_FAIL)
	{
#ifdef _DEBUG
		throw WINDOW_EXCEPTION(GetLastError());
#endif
	}

    // D3D11 리소스를 CUDA에 등록함
    CUresult Result = cuGraphicsD3D11RegisterResource(&CudaResource, _pBuffer, CU_GRAPHICS_REGISTER_FLAGS_NONE);

    //CUdeviceptr DeviceClothPtr;
////= DeformableSurface
    int a = 0;
}

MVertexBuffer::~MVertexBuffer()
{
	SafeRelease(_pBuffer);
}

void MVertexBuffer::setBufferToDevice(UINT &stride, UINT &offset)
{
    g_pGraphicDevice->getContext()->IASetVertexBuffers(0, 1, &_pBuffer, &stride, &offset);
    //g_pGraphicDevice->getContext()->IASetVertexBuffers(0, 1, &_pBuffer, &VertexSize, &offset);
}

ID3D11Buffer* MVertexBuffer::getBuffer()
{
	return _pBuffer;
}

const uint32 MVertexBuffer::getVertexCount() const
{
	return _vertexCount;
}

void MVertexBuffer::UpdateUsingCUDA(PxDeformableSurface* DeformableSurface, uint32 VertexNum)
{
    // CUDA 접근을 위해 리소스를 Map함 
    CUresult Result = cuGraphicsMapResources(1, &CudaResource, cudaStreamDefault);

    // 리소스에 액세스할 수 있는 장치 포인터를 가져옴. 아마 버텍스버퍼의 포인터
    CUdeviceptr DevicePtr;
    size_t  BufSize = 0;
    CUresult Result2 = cuGraphicsResourceGetMappedPointer(&DevicePtr, &BufSize, CudaResource);

    // 버텍스버퍼에 DeformableSurface가 가진 정점들을 복사함
    PxVec4* SimulatedPos = DeformableSurface->getPositionInvMassBufferD();

    std::vector<PxVec4> hostBuf(VertexNum);
    CUdeviceptr devPtr = reinterpret_cast<CUdeviceptr>(DeformableSurface->getPositionInvMassBufferD());
    cuMemcpyDtoH(hostBuf.data(), devPtr, sizeof(PxVec4) * VertexNum);

    //for (int i = 0; i < VertexNum; ++i)
    //{
    //    std::cout << "Begin SimulatedPos: " << hostBuf[i].x << ", " << hostBuf[i].y << ", " << hostBuf[i].z << std::endl;
    //}
    //std::cout << "End SimulatedPos" << std::endl;

    CUstream stream = nullptr;
    cuStreamCreate(&stream, CU_STREAM_DEFAULT);
    CUresult Result3 = cuMemcpyAsync(DevicePtr, reinterpret_cast<CUdeviceptr>(SimulatedPos), sizeof(PxVec4) * VertexNum, stream);
    
    // CUDA 접근이 끝났으니 리소스 UnMap
    CUresult Result4 = cuGraphicsUnmapResources(1, &CudaResource, stream);
}
