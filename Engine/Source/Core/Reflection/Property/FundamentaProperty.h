#pragma once

/*
fundamental 타입인 멤버를 설명하는 구조체.
int, float 등이 해당되며 C배열 타입도 여기에 해당됨.
FPropertyDesc*을 FFundamentalPropertyDesc*<type> 으로 변환하여 Get, Set 기능을 사용할 수 있음.
*/

#include <iostream>
#include "Property.h"

template <class OwnerType, class MemType, class F>
static FPropertyDesc* MakeProp(const std::string& InName, MemType OwnerType::* MemPtr, F InFunc)
{
    // 배열이면 요소 타입의 포인터, 아니면 그대로
    // T[n]         -> T*
    // T*           -> T*
    // T            -> T
    using NoArray = std::conditional_t<
        std::is_array_v<MemType>,
        std::add_pointer_t<std::remove_all_extents_t<MemType>>,
        MemType
    >;

    // 스마트 포인터면 언랩해서 포인터로, 아니면 그대로
    // shared<T>    -> T*
    // T*           -> T*
    // T            -> T
    using NoArrayAndSmart = std::conditional_t<
        is_smart_ptr_v<NoArray>,
        std::add_pointer_t<remove_smart_pointer_t<NoArray>>,
        NoArray
    >;

    // 포인터, 스마트 포인터, 배열 등을 제거한 타입
    using PureType = std::conditional_t<std::is_pointer_v<NoArrayAndSmart>, std::remove_pointer_t<NoArrayAndSmart>, NoArrayAndSmart>;

	struct FPropertyDescImple : public FPropertyDesc
	{
        FPropertyDescImple(MemType OwnerType::* InMemPtr, std::function<void(OwnerType* InObject)> InFunc)
            : MemPtr(InMemPtr), Func(InFunc)
		{}

        // 멤버 포인터
        MemType OwnerType::* MemPtr;

        // 델리게이트
        std::function<void(OwnerType* InObject)> Func;

        virtual bool IsPointer() const
        {
            return std::is_pointer_v<MemType>;
        }

        // 부모 클래스 주석 참고
        virtual void* GetAsVoid(const void* InObject, size_t InIndex = 0) override
        {
            if constexpr (std::is_array_v<MemType>)
            {
                if constexpr (std::rank_v<MemType> == 1)        // 배열
                {
                    return &((OwnerType*)InObject->*MemPtr)[InIndex];
                }
                else if constexpr (std::rank_v<MemType> == 2)   // 행렬 임시 지원
                {
                    return &((OwnerType*)InObject->*MemPtr)[InIndex/4][InIndex%4];
                }
            }
            else
            {
                if (InIndex != 0) 
                { 
                    LOGTEXT(TEXT("배열 멤버가 아님!")); 
                    return nullptr;
                }

                return &((OwnerType*)InObject->*MemPtr);
            }
        }

        // 부모 클래스 주석 참고
        virtual void SetAsVoid(void* InObject, void* InData, size_t InIndex = 0) override
        {
            if constexpr (std::is_array_v<MemType>)
            {
                if constexpr (std::rank_v<MemType> == 1)        // 배열
                {
                    ((OwnerType*)InObject->*MemPtr)[InIndex] = *static_cast<PureType*>(InData);
                }
                else if constexpr (std::rank_v<MemType> == 2)   // 행렬 임시 지원
                {
                    ((OwnerType*)InObject->*MemPtr)[InIndex/4][InIndex%4] = *static_cast<PureType*>(InData);
                }
            }
            else if constexpr (std::is_pointer_v<MemType>)
            {
                // 지원 안함
                ((OwnerType*)InObject->*MemPtr) = static_cast<PureType*>(InData);
            }
            else
            {
                MemType* Data = static_cast<MemType*>(InData);
                if constexpr (is_smart_ptr_v<MemType>)
                {
                    if (InData != nullptr)
                    {
                        ((OwnerType*)InObject->*MemPtr) = *Data;
                    }
                }
                else
                {
                    ((OwnerType*)InObject->*MemPtr) = *Data;
                }
            }

            if (Func)
            {
                Func((OwnerType*)InObject);
            }
        }

        virtual void Copy(const void* InSrcObject, void* InDstObject, size_t InIndex = 0) override
        {
            SetAsVoid(InDstObject, GetAsVoid(InSrcObject, InIndex), InIndex);
        }

        virtual void Delete(void* InData) override
        {
            MemType* Data = static_cast<MemType*>(InData);
            if constexpr (std::is_array_v<MemType>)
            {
                int b = 0;
            }
            else
            {
                delete Data;
            }
        }

        virtual void* GetInstance() override
        {
            return &Instance;
        }

        PureType Instance;
	};
    
    // T[N] -> T
    using Type = std::conditional_t<std::is_array_v<MemType>, std::remove_all_extents_t<MemType>, MemType>;
    std::function<void(OwnerType* InObject)> Func = InFunc;
	FPropertyDesc* NewDesc = new FPropertyDescImple(MemPtr, Func);
	NewDesc->Name = InName;
	NewDesc->Size = sizeof(Type);
	NewDesc->Num = std::is_array_v<MemType> ? sizeof(MemType) / sizeof(Type) : 1;
	NewDesc->Offset = OffsetOf(MemPtr);
    NewDesc->bSharedPtr = is_smart_ptr_v<MemType>;

    SetType<PureType>(NewDesc->Type, NewDesc->TypeDesc);

    std::wstring Msg = TEXT("Make Prop ") + StringToWString(InName);
    LOG(Msg);

	return NewDesc;
}