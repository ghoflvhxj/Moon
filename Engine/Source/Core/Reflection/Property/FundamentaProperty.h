#pragma once

/*
int, float, bool과 같은 근본적인 자료형을 설명하는 구조체
FPropertyDesc*을 FFundamentalPropertyDesc*<type> 으로 변환하여 Get, Set 기능을 사용할 수 있음
*/

#include <iostream>
#include "Property.h"

template <class T>
struct FFundamentalPropertyInterface
{
public:
    // 포인터면 Property 반환이 const&로 반환되도록
    using PropType = std::conditional_t<std::is_pointer_v<T>, std::add_const_t<T>, std::add_lvalue_reference_t<T>>;
    using ElemType = std::conditional_t<std::is_pointer_v<T>, std::remove_pointer_t<T>, T>;
public:
    // 프로퍼티 Getteer
	virtual PropType Get(void* InObject) = 0;
    // 배열 요소 Getter
    virtual ElemType& Get(void* InObject, uint32 InIndex) = 0;
    virtual void Set(void* InObject, const T& InT) = 0;
};

template <class T>
struct FFundamentalPropertyDesc : public FPropertyDesc, public FFundamentalPropertyInterface<T>
{
    // FPropertyDesc->FFundamentalPropertyInterface 로 바로 변환을 못하기 때문에
    // FPropertyDesc->FFundamentalPropertyDesc->FFundamentalPropertyInterface 는 가능
};

// 일반 타입 프로퍼티 생성 함수 템플릿
template <class Owner, class MemType, class F>
static FPropertyDesc* MakeProp(const std::string& InName, MemType Owner::* MemPtr, F InFunc)
{
    // 배열이면 포인터로 변경
    using ImpleType = std::conditional_t<std::is_array_v<MemType>, std::add_pointer_t<std::remove_extent_t<MemType>>, MemType>;
    using ElemType = std::conditional_t<std::is_pointer_v<ImpleType>, std::remove_pointer_t<ImpleType>, ImpleType>;
    // 포인터 -> const T*, 일반 -> T&
    using PropType = std::conditional_t<std::is_pointer_v<ImpleType>, std::add_const_t<ImpleType>, std::add_lvalue_reference_t<ImpleType>>;

	struct FPropertyImple : public FFundamentalPropertyDesc<ImpleType>
	{
        FPropertyImple(MemType Owner::* MemPtr, std::function<void(Owner* InObject)> InFunc)
            : TestMemPtr(MemPtr), Func(InFunc)
		{
		}
        // 멤버 포인터
        MemType Owner::* TestMemPtr;

        // 델리게이트
        std::function<void(Owner* InObject)> Func;

        // Getter
        virtual PropType Get(void* InObject) override
        {
            if constexpr (std::is_array_v<MemType>)
            {
                return &((Owner*)InObject->*TestMemPtr)[0];
            }
            else
            {
                return ((Owner*)InObject->*TestMemPtr);
            }
        }

        // 배열 요소 Getter
		virtual ElemType& Get(void* InObject, uint32 InIndex = 0) override
		{
            if constexpr (std::is_array_v<MemType>)
            {
                return ((Owner*)InObject->*TestMemPtr)[InIndex];
            }
            else
            {
                return ((Owner*)InObject->*TestMemPtr);
            }
		}

        virtual void* GetAsVoid(const void* InObject) override
        {
            return &((Owner*)InObject->*TestMemPtr);
        }

        virtual void Set(void* InObject, const ImpleType& InT) override
        {
            //if constexpr (std::is_array_v<MemType> == false)
            //{
            //    ((Owner*)InObject->*TestMemPtr) = InT;
            //}

            //if (Func)
            //{
            //    Func((Owner*)InObject);
            //}
        }
	};
    
    using Type = std::conditional_t<std::is_array_v<MemType>, std::remove_extent_t<MemType>, MemType>;
    std::function<void(Owner* InObject)> Func = InFunc;
	FPropertyDesc* NewDesc = new FPropertyImple(MemPtr, Func);
	NewDesc->Name = InName;
	NewDesc->Size = sizeof(Type);
	NewDesc->Num = std::is_array_v<MemType> ? sizeof(MemType) / sizeof(Type) : 1;
	NewDesc->bContainer = false;
	NewDesc->Offset = OffsetOf(MemPtr);

    SetType<Type>(NewDesc);

	//cout << "Make Prop" << endl;

	return NewDesc;
}

// 포인터 타입 프로퍼티 생성 함수 템플릿
template <class Owner, class T, class F>
static FPropertyDesc* MakeProp(const std::string& InName, T* Owner::* MemPtr, F InFunc)
{
    struct FPropertyImple : public FFundamentalPropertyDesc<T>
    {
        using ElementType = std::conditional_t<std::is_array_v<T>, std::remove_extent_t<T>, T>;
        FPropertyImple(T Owner::* MemPtr, std::function<void()> InFunc)
            : TestMemPtr(MemPtr), Func(InFunc)
        {
        }
        // 멤버 포인터
        T* Owner::* TestMemPtr;

        // 델리게이트
        std::function<void(Owner* InObject)> Func;

        virtual T& Get(void* InObject) override
        {
            return ((Owner*)InObject->*TestMemPtr);
        }

        virtual ElementType& Get(void* InObject, uint32 InIndex = 0) override
        {
            if constexpr (std::is_array_v<T>)
            {
                return ((Owner*)InObject->*TestMemPtr)[InIndex];
            }
            else
            {
                return ((Owner*)InObject->*TestMemPtr);
            }
        }

        virtual void Set(void* InObject, const T* const InT)
        {
            *((Owner*)InObject->*TestMemPtr) = InT;
            if (Func)
            {
                Func((Owner*)InObject);
            }
        }
    };

    using Type = std::conditional_t<std::is_array_v<T>, std::remove_extent_t<T>, T>;

    std::function<void(Owner* InObject)> Func = InFunc;
    FPropertyDesc* NewDesc = new FPropertyImple(MemPtr, Func);
    NewDesc->Name = InName;
    NewDesc->Size = sizeof(T);
    NewDesc->Num = std::is_array_v<T> ? sizeof(T) / sizeof(Type) : 1;
    NewDesc->bContainer = false;
    NewDesc->Offset = OffsetOf(MemPtr);

    SetType<Type>(NewDesc);
    //cout << "Make Prop" << endl;

    return NewDesc;
}