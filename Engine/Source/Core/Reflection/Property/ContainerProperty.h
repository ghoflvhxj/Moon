#pragma once

#include "Property.h"

using namespace std;

struct FContainerPropertyInterface
{
    // 컨테이너의 키 타입 정보. 컨테이너가 아닌 경우는 nullptr.
    const FTypeDesc* KeyTypeDesc = nullptr;
    EType ContainerKeyType = EType::None;

public:
	virtual void Resize(const void* InObject, const size_t InSize) = 0;
	virtual size_t GetNum(const void* InObject) const = 0;
    virtual void Clear (const void* InObject) = 0;
};

