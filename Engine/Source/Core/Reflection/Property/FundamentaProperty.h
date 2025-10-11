#pragma once

/*
fundamental 타입인 멤버를 설명하는 구조체.
int, float 등이 해당되며 C배열 타입도 여기에 해당됨.
FPropertyDesc*을 FFundamentalPropertyDesc*<type> 으로 변환하여 Get, Set 기능을 사용할 수 있음.
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

    // Setter
    virtual void Set(void* InObject, const T& InT) = 0;
    virtual void Set(void* InObject, std::shared_ptr<ElemType> InT) {}
};

template <class T>
struct FFundamentalPropertyDesc : public FPropertyDesc, public FFundamentalPropertyInterface<T>
{
    // 캐스팅 용 Wrapper
};

template <class Owner, class MemType, class F>
static FPropertyDesc* MakeProp(const std::string& InName, MemType Owner::* MemPtr, F InFunc)
{
    // 배열이면 요소 타입의 포인터, 아니면 그대로
    // T[n]         -> T*
    // T*           -> T*
    // T            -> T
    using _NoArray = std::conditional_t<
        std::is_array_v<MemType>,
        std::add_pointer_t<std::remove_all_extents_t<MemType>>,
        MemType
    >;

    // 스마트 포인터면 언랩해서 포인터로, 아니면 그대로
    // shared<T>    -> T*
    // T*           -> T*
    // T            -> T
    using _NoSmart = std::conditional_t<
        is_smart_ptr_v<_NoArray>,
        std::add_pointer_t<remove_smart_pointer_t<_NoArray>>,
        _NoArray
    >;

    // 순수 또는 포인터 타입
    using ImpleType = _NoSmart;

    // 포인터 -> T*, 일반 -> T&
    using PropType = std::conditional_t<std::is_pointer_v<ImpleType>, ImpleType, std::add_lvalue_reference_t<ImpleType>>;

    // 포인터, 스마트 포인터, 배열 등을 제거한 타입
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

  //      virtual PropType Get(const void* InObject) override
  //      {
  //          if constexpr (std::is_array_v<MemType>)
  //          {
  //              return ((Owner*)InObject->*TestMemPtr);
  //          }
  //          else if constexpr (is_smart_ptr_v<MemType>)
  //          {
  //              return ((Owner*)InObject->*TestMemPtr).get();
  //          }
  //          else
  //          {
  //              return ((Owner*)InObject->*TestMemPtr);
  //          }
  //      }

  //      // 타입이 지정된 Getter
		//virtual ElemType& Get(const void* InObject, size_t InIndex = 0) override
		//{
  //          if constexpr (std::is_array_v<MemType>)
  //          {
  //              return ((Owner*)InObject->*TestMemPtr)[InIndex];
  //          }
  //          else if constexpr (is_smart_ptr_v<MemType>)
  //          {
  //              return *((Owner*)InObject->*TestMemPtr).get();
  //          }
  //          else
  //          {
  //              return ((Owner*)InObject->*TestMemPtr);
  //          }
		//}

        // 부모 클래스 주석 참고
        virtual void* GetAsVoid(const void* InObject, size_t InIndex = 0) override
        {
            if constexpr (is_smart_ptr_v<MemType>)
            {
                if (InIndex != 0) { MSGBOX(TEXT("배열 멤버가 아님!")); }
                return &((Owner*)InObject->*TestMemPtr);
            }
            else if constexpr (std::is_array_v<MemType>)
            {
                if constexpr (std::rank_v<MemType> == 1)
                {
                    return &((Owner*)InObject->*TestMemPtr)[InIndex];
                }
                else if constexpr (std::rank_v<MemType> == 2)
                {
                    return &((Owner*)InObject->*TestMemPtr)[InIndex/4][InIndex%4];
                }
            }
            else
            {
                if (InIndex != 0) { MSGBOX(TEXT("배열 멤버가 아님!")); }
                return &((Owner*)InObject->*TestMemPtr);
            }
        }

        // 부모 클래스 주석 참고
        virtual void SetAsVoid(void* InObject, void*& InData, size_t InIndex = 0) override
        {
            if constexpr (is_smart_ptr_v<MemType>)
            {
                MemType* Data = static_cast<MemType*>(InData);
                ((Owner*)InObject->*TestMemPtr) = *Data;
            }
            else if constexpr (std::is_pointer_v<MemType>)
            {
                // 지원 안함
                InData = nullptr;
            }
            else if constexpr (std::is_array_v<MemType>)
            {
                if constexpr (std::rank_v<MemType> == 1)
                {
                    ((Owner*)InObject->*TestMemPtr)[InIndex] = *static_cast<ElemType*>(InData);
                }
                else if constexpr (std::rank_v<MemType> == 2)
                {
                    ((Owner*)InObject->*TestMemPtr)[InIndex/4][InIndex%4] = *static_cast<ElemType*>(InData);
                }
            }
            else
            {
                // 복사 대입
                MemType* Ptr = static_cast<MemType*>(InData);
                ((Owner*)InObject->*TestMemPtr) = *Ptr;
                delete Ptr;
                InData = nullptr;
            }
        }

        virtual void Set(void* InObject, const ImpleType& InT) override
        {
            if constexpr (std::is_array_v<MemType>)
            {
                // DoNothing
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

        virtual void Alloc(void* InObject)
        {
            if constexpr (is_smart_ptr_v<MemType>)
            {
                ((Owner*)InObject->*TestMemPtr) = std::make_shared<ElemType>();
            }
            else if constexpr (std::is_pointer_v<MemType>)
            {
                ((Owner*)InObject->*TestMemPtr) = new MemType;
            }
            else
            {
                // DoNothing
            }
        }
	};
    
    // T[N] -> T
    using Type = std::conditional_t<std::is_array_v<MemType>, std::remove_all_extents_t<MemType>, MemType>;
    std::function<void(Owner* InObject)> Func = InFunc;
	FPropertyDesc* NewDesc = new FPropertyImple(MemPtr, Func);
	NewDesc->Name = InName;
	NewDesc->Size = sizeof(Type);
	NewDesc->Num = std::is_array_v<MemType> ? sizeof(MemType) / sizeof(Type) : 1;
	NewDesc->Offset = OffsetOf(MemPtr);
    NewDesc->bSharedValue = is_smart_ptr_v<MemType>;

    SetType<ElemType>(NewDesc->Type, NewDesc->TypeDesc);

    std::wstring Msg = TEXT("Make Prop ") + StringToWString(InName);
    LOG(Msg);

	return NewDesc;
}