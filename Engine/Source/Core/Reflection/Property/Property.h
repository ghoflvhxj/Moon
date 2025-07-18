#pragma once

#include <iostream>
#include <vector>
#include "Type.h"

using namespace std;

struct FTypeDesc;

// 클래스나 구조체의 멤버 정보를 담음
struct FPropertyDesc
{
	std::string Name;
	size_t Offset;
	size_t Size;
	size_t Num;
	EType Type;

	// 프로퍼티가 vector, map 같은 컨테이너 인지?
	bool bContainer;
	// 프로퍼티가 포인터를 저장하는 컨테이너 인지?
	bool bPointerElements;

	const FTypeDesc* TypeDesc = nullptr;

	// 이 프로퍼티가 저장한 항목들의 타입 정보
	// ex) vector<A*> = { A, B, ... } 같이 다형성으로 인해 항목들이 정보가 다를 수 있음.
	std::vector<const FTypeDesc*> ElementsDesc;

public:
	virtual size_t GetSize() 
    { 
        return Size; 
    }

    virtual void* GetAsVoid(void* InObject)
    {
        return nullptr;
    }
};

template <class ClassType, class Type>
constexpr size_t OffsetOf(Type ClassType::* Ptr)
{
    return reinterpret_cast<size_t>(
        &(reinterpret_cast<ClassType*>(0)->*Ptr)
        );
}