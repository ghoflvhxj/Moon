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
};

struct FContainerPropertyDesc : public FPropertyDesc, public FContainerPropertyInterface
{
};

// 일반 타입 프로퍼티 함수 템플릿
template <class Owner, class T, class F >
static FPropertyDesc* MakeProp(const std::string& InName, std::vector<T> Owner::* MemPtr, F InFunc)
{
	struct FContainerDescImple : public FContainerPropertyDesc
	{
        FContainerDescImple(std::vector<T> Owner::* MemPtr, std::function<void(Owner* InObject)> InFunc)
            : TestMemPtr(MemPtr), Func(InFunc)
        {
        }
        std::vector<T> Owner::* TestMemPtr;
        std::function<void(Owner* InObject)> Func;

		virtual void Resize(void* InObject, const size_t InSize) override
		{
			((Owner*)InObject->*TestMemPtr).resize(InSize);
		}

		virtual size_t GetNum(void* InObject) const override
		{
			return ((Owner*)InObject->*TestMemPtr).size();
		}

		virtual void* Get(void* InObject, const size_t InIndex) override
		{
			return &((Owner*)InObject->*TestMemPtr)[InIndex];
		}

        virtual void* GetAsVoid(const void* InObject) override
        {
            return &((Owner*)InObject->*TestMemPtr);
        }

        virtual void Set(void* InObject, const size_t InIndex, void* InData) override
        {
            ((Owner*)InObject->*TestMemPtr)[InIndex] = *static_cast<T*>(InData);
            if (Func)
            {
                Func((Owner*)InObject);
            }
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

	//cout << "Make Vec Prop NonPointer" << endl;

	return NewDesc;
}

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
            ((Owner*)InObject->*TestMemPtr).resize(InSize);
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
            //((Owner*)InObject->*TestMemPtr)[InIndex] = *static_cast<T*>(InData);
            //if (Func)
            //{
            //    Func((Owner*)InObject);
            //}
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

template <class Owner, class T>
static FPropertyDesc* MakeProp(const std::string& InName, std::vector<T*> Owner::* MemPtr, std::function<void(Owner* InObject)> InFunc)
{
	FPropertyDesc* NewDesc = new FPropertyDesc;
	NewDesc->Name = InName;
	NewDesc->Size = sizeof(T);
	//NewDesc->Offset = InOffset;
	NewDesc->bContainer = true;
	NewDesc->TypeDesc = &T::GetTypeDescStatic();
	NewDesc->bPointerElements = true;

	//cout << "Make Vec Prop Pointer" << endl;

	// static 시점에 아직 데이터가 안들어가 있으니, 알 방법이 없음
	//for (int i = 0; i < NewDesc.Num; ++i)
	//{
	//	if constexpr (std::is_fundamental_v<T> == false)
	//	{
	//		NewDesc.ElementsDesc.push_back(&InT[i]->GetTypeDesc());
	//	}
	//}

	return NewDesc;
}
