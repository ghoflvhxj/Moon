#pragma once

#include <iostream>
#include "Property.h"

template <class T>
struct FFundamentalPropertyInterface
{
public:
	virtual T& Get(void* InObject) = 0;
    virtual void Set(void* InObject, const T& InT) = 0;
};

// dynamic_cast를 피하기 위해서
// FPropertyDesc->FFundamentalPropertyInterface 로 바로 변환은 못함
// FPropertyDesc->FFundamentalPropertyDesc->FFundamentalPropertyInterface 는 가능
template <class T>
struct FFundamentalPropertyDesc : public FPropertyDesc, public FFundamentalPropertyInterface<T>
{

};

// 일반 타입 프로퍼티 생성 함수 템플릿
template <class Owner, class T, class F>
static FPropertyDesc* MakeProp(const std::string& InName, T Owner::* MemPtr, F InFunc)
{
	struct FPropertyImple : public FFundamentalPropertyDesc<T>
	{
        FPropertyImple(T Owner::* MemPtr, std::function<void(Owner* InObject)> InFunc)
            : TestMemPtr(MemPtr), Func(InFunc)
		{
		}
        // 멤버 포인터
        T Owner::* TestMemPtr;

        // 델리게이트
        std::function<void(Owner* InObject)> Func;

		virtual T& Get(void* InObject) override
		{
			return ((Owner*)InObject->*TestMemPtr);
		}

        virtual void* GetAsVoid(void* InObject) override
        {
            return &((Owner*)InObject->*TestMemPtr);
        }

        virtual void Set(void* InObject, const T& InT) override
        {
            if constexpr (std::is_array_v<T> == false)
            {
                ((Owner*)InObject->*TestMemPtr) = InT;
            }

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
	NewDesc->Size = sizeof(Type);
	NewDesc->Num = 1;
	NewDesc->bContainer = false;
	NewDesc->Offset = OffsetOf(MemPtr);


	if constexpr (std::is_same_v<int, Type>)
	{
		NewDesc->Type = EType::Int;
	}
	else if constexpr (std::is_same_v<float, Type>)
	{
		NewDesc->Type = EType::Float;
	}
	else if constexpr (std::is_same_v<bool, Type>)
	{
		NewDesc->Type = EType::Bool;
	}
    else if constexpr (std::is_same_v<::Vec2, Type>)
    {
        NewDesc->Type = EType::Vec2;
    }
    else if constexpr (std::is_same_v<::Vec3, Type>)
    {
        NewDesc->Type = EType::Vec3;
    }
    else if constexpr (std::is_same_v<::Vec4, Type>)
    {
        NewDesc->Type = EType::Vec4;
    }
	else if constexpr (std::is_fundamental_v<Type> == false)
	{
		NewDesc->TypeDesc = &T::GetTypeDescStatic();
	}

	//cout << "Make Prop" << endl;

	return NewDesc;
}

// 포인터 타입 프로퍼티 생성 함수 템플릿
template <class Owner, class T, class F>
static FPropertyDesc* MakeProp(const std::string& InName, T* Owner::* MemPtr, F InFunc)
{
    struct FPropertyImple : public FFundamentalPropertyDesc<T>
    {
        FPropertyImple(T Owner::* MemPtr, std::function<void()> InFunc)
            : TestMemPtr(MemPtr), Func(InFunc)
        {
        }
        // 멤버 포인터
        T* Owner::* TestMemPtr;

        // 델리게이트
        std::function<void(Owner* InObject)> Func;

        virtual T* Get(void* InObject) override
        {
            return ((Owner*)InObject->*TestMemPtr);
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

    std::function<void(Owner* InObject)> Func = InFunc;
    FPropertyDesc* NewDesc = new FPropertyImple(MemPtr, Func);
    NewDesc->Name = InName;
    NewDesc->Size = sizeof(T);
    NewDesc->Num = 1;
    NewDesc->bContainer = false;
    NewDesc->Offset = OffsetOf(MemPtr);

    if constexpr (std::is_same_v<int, T>)
    {
        NewDesc->Type = EType::Int;
    }
    else if constexpr (std::is_same_v<float, T>)
    {
        NewDesc->Type = EType::Float;
    }
    else if constexpr (std::is_same_v<double, T>)
    {
        NewDesc->Type = EType::Double;
    }
    else if constexpr (std::is_same_v<Vec3, T>)
    {
        NewDesc->Type = EType::Vec3;
    }
    else if constexpr (std::is_fundamental_v<T> == false)
    {
        NewDesc->TypeDesc = &T::GetTypeDescStatic();
    }

    //cout << "Make Prop" << endl;

    return NewDesc;
}