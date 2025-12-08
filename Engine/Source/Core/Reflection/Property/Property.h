#pragma once

#include <iostream>
#include <vector>
#include <functional>
#include <map>
#include <unordered_map>

#include "Type.h"
#include "Core/Reflection/TypeDesc.h"
#include "Macro.h"

using namespace std;

struct FTypeDesc;

// 클래스나 구조체의 멤버 정보를 담음
struct FPropertyDesc
{
	std::string Name;
	size_t Offset = 0;
	size_t Size = 0;
	size_t Num = 0;

	// 프로퍼티가 vector, map 같은 컨테이너 인지?
    EContainerType ContainerType = EContainerType::None;

	// 프로퍼티가 shared_ptr 인지?
    bool bSharedPtr = false;

    // 이 프로퍼티의 TypeDesc. 컨테이너인 경우는 요소의 TypeDesc.
	const FTypeDesc* TypeDesc = nullptr;
	EType Type = EType::None;

public:
    virtual ~FPropertyDesc() = default;

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
        return ContainerType != EContainerType::None;
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

    // 프로퍼티를 void* 를 이용해 설정하는 함수
    virtual void SetAsVoid(void* InObject, void* InData, size_t InIndex = 0)
    {

    }

    virtual void Copy(const void* InSrcObject, void* InDstObject, size_t InIndex = 0)
    {

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

template <class T>
struct is_map : std::false_type {};

template <class T, class V>
struct is_map<std::map<T, V>> : std::true_type {};

template <class T, class V>
inline constexpr bool is_map_v = is_map<T, V>::value;

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

template <class T>
void SetType(EType& InType, const FTypeDesc* &InTypeDesc)
{
    if constexpr (std::is_same_v<int, T> || std::is_same_v<uint32, T>)
    {
        InType = EType::Int;
    }
    else if constexpr (std::is_same_v<float, T>)
    {
        InType = EType::Float;
    }
    else if constexpr (std::is_same_v<bool, T>)
    {
        InType = EType::Bool;
    }
    else if constexpr (std::is_same_v<::Vec2, T>)
    {
        InType = EType::Vec2;
    }
    else if constexpr (std::is_same_v<::Vec3, T>)
    {
        InType = EType::Vec3;
    }
    else if constexpr (std::is_same_v<::Vec4, T>)
    {
        InType = EType::Vec4;
    }
    else if constexpr (std::is_same_v<std::string, T>)
    {
        InType = EType::String;
    }
    else if constexpr (std::is_same_v<std::wstring, T>)
    {
        InType = EType::WString;
    }
    else if constexpr (std::is_same_v<::Mat4, T>)
    {
        InType = EType::Mat4;
    }
    else if constexpr (std::is_enum_v<T>)
    {
        InType = EType::Enum;
    }
    else if constexpr (std::is_fundamental_v<T> == false)
    {
        InTypeDesc = GetTypeDesc<T>();

        // 외부 타입일 경우 매번 특수화한 함수에 작성해야 하는데, 이 코드로 피할 수 있음.
        AddTypeDesc<T>(InTypeDesc);
    }
}