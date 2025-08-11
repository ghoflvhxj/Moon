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

    // 이 프로퍼티의 TypeDesc. 컨테이너인 경우는 요소의 TypeDesc.
	const FTypeDesc* TypeDesc = nullptr;

	// 이 프로퍼티가 저장한 항목들의 타입 정보
	// ex) vector<A*> = { A, B, ... } 같이 다형성으로 인해 항목들이 정보가 다를 수 있음.
	std::vector<const FTypeDesc*> ElementsDesc;

public:
    std::string GetDisplayName() const
    {
        if (IsContainer())
        {
            return Name + "- Container";
        }
        else if (IsArray())
        {
            return Name + "- Array";
        }
        else
        {
            return Name;
        }
    }

    bool IsContainer() const
    {
        return bContainer;
    }

    bool IsArray() const
    {
        return Num > 1;
    }

public:
	virtual size_t GetSize() 
    { 
        return Size; 
    }

    // 프로퍼티를 void* 로 반환하는 함수
    virtual void* GetAsVoid(const void* InObject, size_t InIndex = 0)
    {
        return nullptr;
    }

    template <class T>
    bool IsA()
    {
        const FTypeDesc* Target = T::GetTypeDescStatic();
        const FTypeDesc* Current = TypeDesc;
        while (Current)
        {
            if (Current == Target)
            {
                return true;
            }

            Current = Current->Parent;
        }

        return false;
    }
};

template <class ClassType, class Type>
constexpr size_t OffsetOf(Type ClassType::* Ptr)
{
    return reinterpret_cast<size_t>(
        &(reinterpret_cast<ClassType*>(0)->*Ptr)
    );
}

template<typename T>
struct is_smart_ptr : std::false_type {};

// shared_ptr 특수화
template<typename U>
struct is_smart_ptr<std::shared_ptr<U>> : std::true_type {};

template<typename T>
inline constexpr bool is_smart_ptr_v = is_smart_ptr<T>::value;

template<typename T>
struct remove_smart_pointer {
    using type = T;
};

template<typename U>
struct remove_smart_pointer<std::shared_ptr<U>> {
    using type = U;
};

template<typename T>
using remove_smart_pointer_t = typename remove_smart_pointer<T>::type;

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