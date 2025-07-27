#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include "Property.h"

using namespace std;

struct FContainerPropertyInterface
{
public:
	virtual void Resize(void* InObject, const size_t InSize) = 0;
	virtual size_t GetNum(void* InObject) const = 0;
	virtual void* Get(void* InObject, const size_t InIndex) = 0;
    virtual void Set(void* InObject, const size_t InIndex, void* InData) = 0;
    virtual void Clear (void* InObject) = 0;
};

struct FContainerPropertyDesc : public FPropertyDesc, public FContainerPropertyInterface
{
};

// 일반 타입 프로퍼티 함수 템플릿
template <class Owner, class MemType, class F >
static FPropertyDesc* MakeProp(const std::string& InName, std::vector<MemType> Owner::* MemPtr, F InFunc)
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

    // 최종적으로 ImpleType
    using ImpleType = _NoSmart;

    // 포인터 -> T*, 일반 -> T&
    using PropType = std::conditional_t<std::is_pointer_v<ImpleType>, ImpleType, std::add_lvalue_reference_t<ImpleType>>;

    // 포인터, 스마트 포인터, 배열 등을 제거한 순수 타입
    using ElemType = std::conditional_t<std::is_pointer_v<ImpleType>, std::remove_pointer_t<ImpleType>, ImpleType>;

	struct FContainerDescImple : public FContainerPropertyDesc
	{
        FContainerDescImple(std::vector<MemType> Owner::* MemPtr, std::function<void(Owner* InObject)> InFunc)
            : TestMemPtr(MemPtr), Func(InFunc)
        {
        }
        std::vector<MemType> Owner::* TestMemPtr = nullptr;
        std::function<void(Owner* InObject)> Func;

		virtual void Resize(void* InObject, const size_t InSize) override
        {
            if constexpr (is_smart_ptr_v<MemType>)
            {
                for (size_t i = GetNum(InObject); i < InSize; ++i)
                {
                    ((Owner*)InObject->*TestMemPtr).push_back(std::make_shared<ElemType>()); // 굳이 생성까지 해줘야 하나? 쓰는 쪽에서 넣어줘야 하는게 맞는듯?
                }
            }
            else
            {
                auto& Vec = ((Owner*)InObject->*TestMemPtr);
                Vec.resize(InSize);
            }
		}

		virtual size_t GetNum(void* InObject) const override
		{
			return ((Owner*)InObject->*TestMemPtr).size();
		}

		virtual void* Get(void* InObject, const size_t InIndex) override
		{
            if constexpr (std::is_pointer_v<MemType>)
            {
                return ((Owner*)InObject->*TestMemPtr)[InIndex];
            }
            else if constexpr (is_smart_ptr_v<MemType>)
            {
                auto& Vec = ((Owner*)InObject->*TestMemPtr);
                if (((Owner*)InObject->*TestMemPtr)[InIndex])
                {
                    return ((Owner*)InObject->*TestMemPtr)[InIndex].get();
                }
                else
                {
                    return nullptr;
                }
            }
            else
            {
			    return &((Owner*)InObject->*TestMemPtr)[InIndex];
            }
		}

        virtual void* GetAsVoid(const void* InObject) override
        {
            if constexpr (std::is_pointer_v<MemType>)
            {
                return ((Owner*)InObject->*TestMemPtr);
            }
            else
            {
                return &((Owner*)InObject->*TestMemPtr);
            }
        }

        virtual void Set(void* InObject, const size_t InIndex, void* InData) override
        {
            //((Owner*)InObject->*TestMemPtr)[InIndex] = *static_cast<T*>(InData);
            //if (Func)
            //{
            //    Func((Owner*)InObject);
            //}
        }

        virtual void Clear(void* InObject) override
        {
            ((Owner*)InObject->*TestMemPtr).clear();
        }
	};

    using Type = std::conditional_t<std::is_array_v<MemType>, std::remove_extent_t<MemType>, MemType>;

    std::function<void(Owner* InObject)> Func = InFunc;
	FPropertyDesc* NewDesc = new FContainerDescImple(MemPtr, Func);
	NewDesc->Name = InName;
	NewDesc->Size = sizeof(ElemType);
	//NewDesc->Offset = InOffset;
	NewDesc->bContainer = true;
	NewDesc->bPointerElements = false;

    SetType<ElemType>(NewDesc);

	//cout << "Make Vec Prop NonPointer" << endl;

	return NewDesc;
}

/*
// 스마트 포인터 타입 벡터 템플릿
template <class Owner, class T>
static FPropertyDesc* MakeProp(const std::string& InName, std::vector<std::shared_ptr<T>> Owner::* MemPtr, std::function<void(Owner* InObject)> InFunc)
{
    struct FContainerDescImple : public FContainerPropertyDesc
    {
        FContainerDescImple(std::vector<std::shared_ptr<T>> Owner::* MemPtr, std::function<void(Owner* InObject)> InFunc)
            : TestMemPtr(MemPtr), Func(InFunc)
        {
        }
        std::vector<std::shared_ptr<T>> Owner::* TestMemPtr;
        std::function<void(Owner* InObject)> Func;

        virtual void Resize(void* InObject, const size_t InSize) override
        {
            for (size_t i = GetNum(InObject); i < InSize; ++i)
            {
                ((Owner*)InObject->*TestMemPtr).push_back(std::make_shared<T>());
            }
        }

        virtual size_t GetNum(void* InObject) const override
        {
            return ((Owner*)InObject->*TestMemPtr).size();
        }

        virtual void* Get(void* InObject, const size_t InIndex) override
        {
            if (((Owner*)InObject->*TestMemPtr)[InIndex])
            {
                return ((Owner*)InObject->*TestMemPtr)[InIndex].get();
            }

            return nullptr;
        }

        virtual void* GetAsVoid(const void* InObject) override
        {
            return &((Owner*)InObject->*TestMemPtr);
        }

        virtual void Set(void* InObject, const size_t InIndex, void* InData) override
        {
            *((Owner*)InObject->*TestMemPtr)[InIndex] = *static_cast<T*>(InData);
            if (Func)
            {
                Func((Owner*)InObject);
            }
        }

        virtual void Clear(void* InObject) override
        {
            ((Owner*)InObject->*TestMemPtr).clear();
        }
    };

    using Type = T;

    std::function<void(Owner* InObject)> Func = InFunc;
    FPropertyDesc* NewDesc = new FContainerDescImple(MemPtr, Func);
    NewDesc->Name = InName;
    NewDesc->Size = sizeof(T);
    //NewDesc->Offset = InOffset;
    NewDesc->bContainer = true;
    NewDesc->bPointerElements = false;

    SetType<Type>(NewDesc);

    return NewDesc;
}
*/