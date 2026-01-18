#include "StructuredBuffer.h"
#include "GraphicDevice.h"

void MBuffer::Init(const uint32 InElementSize, const uint32 InElementNum)
{
    ElementSize = InElementSize;
    ElementNum = InElementNum;

    BufferSize = ElementNum * ElementSize;
}
