#pragma once

#include <iostream>
#include <vector>
#include "Type.h"
#include "TypeDesc.h"

using namespace std;

struct FTypeDesc;

// 클래스나 구조체의 멤버 정보를 담음
struct FPropertyDesc
{
	std::string Name;
	size_t Offset = 0;
	size_t Size = 0;
	size_t Num = 0;
	EType Type = EType::None;

	// 프로퍼티가 vector, map 같은 컨테이너 인지?
	bool bContainer = false;

	// 프로퍼티가 포인터를 저장하는 컨테이너 인지?
	bool bPointerElements = false;

	const FTypeDesc* TypeDesc = nullptr;

	// 이 프로퍼티가 저장한 항목들의 타입 정보
	// ex) vector<A*> = { A, B, ... } 같이 다형성으로 인해 항목들이 정보가 다를 수 있음.
	std::vector<const FTypeDesc*> ElementsDesc;

public:
	virtual size_t GetSize() 
    { 
        return Size; 
    }

    virtual void* GetAsVoid(const void* InObject)
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

template <class Type>
void SetType(FPropertyDesc* InDesc)
{
    if constexpr (std::is_same_v<int, Type> || std::is_same_v<uint32, Type>)
    {
        InDesc->Type = EType::Int;
    }
    else if constexpr (std::is_same_v<float, Type>)
    {
        InDesc->Type = EType::Float;
    }
    else if constexpr (std::is_same_v<bool, Type>)
    {
        InDesc->Type = EType::Bool;
    }
    else if constexpr (std::is_same_v<::Vec2, Type>)
    {
        InDesc->Type = EType::Vec2;
    }
    else if constexpr (std::is_same_v<::Vec3, Type>)
    {
        InDesc->Type = EType::Vec3;
    }
    else if constexpr (std::is_same_v<::Vec4, Type>)
    {
        InDesc->Type = EType::Vec4;
    }
    else if constexpr (std::is_same_v<std::string, Type>)
    {
        InDesc->Type = EType::String;
    }
    else if constexpr (std::is_same_v<std::wstring, Type>)
    {
        InDesc->Type = EType::WString;
    }
    else if constexpr (std::is_enum_v<Type>)
    {
        InDesc->Type = EType::Enum;
    }
    else if constexpr (std::is_fundamental_v<Type> == false)
    {
        InDesc->TypeDesc = GetTypeDesc<Type>();
    }
}