#pragma once

/*
FPropertyDesc*을 FFundamentalPropertyDesc*<type> 으로 변환하여 Get, Set 기능을 사용할 수 있음
*/

#include <iostream>
#include "Property.h"

template <class T>
struct FFundamentalPropertyInterface
{
public:
    // 값이면 Property Getter가 레퍼런스 타입을 반환하도록
    using PropType = std::conditional_t<std::is_pointer_v<T>, T, std::add_lvalue_reference_t<T>>;
    using ElemType = std::conditional_t<std::is_pointer_v<T>, std::remove_pointer_t<T>, T>;
public:
    // 프로퍼티 자체를 얻는 Getter
	virtual PropType Get(void* InObject) = 0;
    // 배열 요소 Getter
    virtual ElemType& Get(void* InObject, uint32 InIndex) = 0;
    virtual void Set(void* InObject, const T& InT) = 0;

    virtual void Set(void* InObject, std::shared_ptr<ElemType> InT) {}
};

template <class T>
struct FFundamentalPropertyDesc : public FPropertyDesc, public FFundamentalPropertyInterface<T>
{
    // FPropertyDesc->FFundamentalPropertyInterface 로 바로 변환을 못하기 때문에
    // FPropertyDesc->FFundamentalPropertyDesc->FFundamentalPropertyInterface 는 가능
};

// 단일 타입, 배열 타입 프로퍼티 생성 함수 템플릿
template <class Owner, class MemType, class F>
static FPropertyDesc* MakeProp(const std::string& InName, MemType Owner::* MemPtr, F InFunc)
{
    // 1) 배열이면 요소 타입의 포인터, 아니면 그대로
    using _NoArray = std::conditional_t<
        std::is_array_v<MemType>,
        std::add_pointer_t<std::remove_extent_t<MemType>>,
        MemType
    >;

    // 2) 스마트 포인터면 언랩해서 포인터로, 아니면 그대로
    using _NoSmart = std::conditional_t<
        is_smart_ptr_v<_NoArray>,
        std::add_pointer_t<remove_smart_pointer_t<_NoArray>>,
        _NoArray
    >;

    // 순수 또는 포인터 타입
    using ImpleType = _NoSmart;

    // 포인터 -> T*, 일반 -> T&
    using PropType = std::conditional_t<std::is_pointer_v<ImpleType>, ImpleType, std::add_lvalue_reference_t<ImpleType>>;

    // 포인터, 스마트 포인터, 배열 등을 제거한 순수 타입
    using ElemType = std::conditional_t<std::is_pointer_v<ImpleType>, std::remove_pointer_t<ImpleType>, ImpleType>;

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

        virtual PropType Get(void* InObject) override
        {
            if constexpr (std::is_array_v<MemType>)
            {
                return &((Owner*)InObject->*TestMemPtr)[0];
            }
            else if constexpr (is_smart_ptr_v<MemType>)
            {
                return ((Owner*)InObject->*TestMemPtr).get();
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
            else if constexpr (is_smart_ptr_v<MemType>)
            {
                return *((Owner*)InObject->*TestMemPtr).get();
            }
            else
            {
                return ((Owner*)InObject->*TestMemPtr);
            }
		}

        virtual void* GetAsVoid(const void* InObject, size_t InIndex = 0) override
        {
            if constexpr (std::is_array_v<MemType>)
            {
                return &((Owner*)InObject->*TestMemPtr)[InIndex];
            }
            if constexpr (is_smart_ptr_v<MemType>)
            {
                if (InIndex != 0) { MSGBOX(TEXT("배열 멤버가 아님!")); }
                return ((Owner*)InObject->*TestMemPtr).get();
            }
            else
            {
                if (InIndex != 0) { MSGBOX(TEXT("배열 멤버가 아님!")); }
                return &((Owner*)InObject->*TestMemPtr);
            }
        }

        virtual void Set(void* InObject, const ImpleType& InT) override
        {
            if constexpr (std::is_array_v<MemType>)
            {
                
            }
            else if constexpr (is_smart_ptr_v<MemType>)
            {
                ((Owner*)InObject->*TestMemPtr).reset(InT);
            }
            else
            {
                ((Owner*)InObject->*TestMemPtr) = InT;
            }

            if (Func)
            {
                Func((Owner*)InObject);
            }
        }

        virtual void Set(void* InObject, std::shared_ptr<ElemType> InT) override
        {
            if constexpr (is_smart_ptr_v<MemType>)
            {
                ((Owner*)InObject->*TestMemPtr) = InT;
            }

            if (Func)
            {
                Func((Owner*)InObject);
            }
        }
	};
    
    // T[N] -> T
    using Type = std::conditional_t<std::is_array_v<MemType>, std::remove_extent_t<MemType>, MemType>;
    std::function<void(Owner* InObject)> Func = InFunc;
	FPropertyDesc* NewDesc = new FPropertyImple(MemPtr, Func);
	NewDesc->Name = InName;
	NewDesc->Size = sizeof(Type);
	NewDesc->Num = std::is_array_v<MemType> ? sizeof(MemType) / sizeof(Type) : 1;
	NewDesc->bContainer = false;
	NewDesc->Offset = OffsetOf(MemPtr);

    SetType<ElemType>(NewDesc);

	//cout << "Make Prop" << endl;

	return NewDesc;
}