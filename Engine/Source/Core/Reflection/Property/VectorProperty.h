#pragma once

#include "ContainerProperty.h"
#include <vector>

struct FVectorPropertyDesc : public FPropertyDesc, public FContainerPropertyInterface
{
    virtual void* Get(const void* InObject, const size_t InIndex) = 0;
    virtual void Set(const void* InObject, const size_t InIndex, void* InData) = 0;
    virtual void PushBack(const void* InObject, void*& InData) = 0;
    virtual void Reserve(const void* InObject, const size_t InCapacity) = 0;
};

template <class OwnerType, class ElemType, class F >
static FPropertyDesc* MakeProp(const std::string& InName, std::vector<ElemType> OwnerType::* MemPtr, F InFunc)
{
    // 스마트 포인터면 언랩해서 포인터로, 아니면 그대로
    using NoSmartElemType = std::conditional_t<
        is_smart_ptr_v<ElemType>,
        std::add_pointer_t<remove_smart_pointer_t<ElemType>>,
        ElemType
    >;

    // 스마트 포인터 제거 타입
    // T*               -> T*
    // shared_ptr<T>    -> T*
    using ImpleType = NoSmartElemType;

    // 포인터, 스마트 포인터, 배열 등을 제거한 순수 타입
    // T*               -> T
    // shared_ptr<T>    -> T
    using PureType = std::conditional_t<std::is_pointer_v<ImpleType>, std::remove_pointer_t<ImpleType>, ImpleType>;

    struct FContainerDescImple : public FVectorPropertyDesc
    {
        FContainerDescImple(std::vector<ElemType> OwnerType::* MemPtr, std::function<void(OwnerType* InObject)> InFunc)
            : TestMemPtr(MemPtr), Func(InFunc)
        {
        }
        std::vector<ElemType> OwnerType::* TestMemPtr = nullptr;
        std::function<void(OwnerType* InObject)> Func;
        ElemType ValueInstance;

        std::vector<ElemType>& GetVector(const void* InObject) const
        {
            return (OwnerType*)InObject->*TestMemPtr;
        }

        // --------------------------------------------------------------------------------------------
        // FProperty
        virtual void* GetAsVoid(const void* InObject, size_t InIndex = 0) override
        {
            return &GetVector(InObject);
        }

        virtual void Copy(const void* InSrcObject, void* InDstObject, size_t InIndex = 0) override
        {
            auto& SrcVector = GetVector(InSrcObject);
            auto& DstVecotr = GetVector(InDstObject);

            DstVecotr.clear();
            DstVecotr.assign(SrcVector.begin(), SrcVector.end());
        }

        // --------------------------------------------------------------------------------------------
        // FContainerProperty
        virtual void Resize(const void* InObject, const size_t InSize) override
        {
            auto& Vector = GetVector(InObject);
            if constexpr (is_smart_ptr_v<ElemType>)
            {
                for (size_t i = GetNum(InObject); i < InSize; ++i)
                {
                    Vector.push_back(std::make_shared<PureType>());
                }
            }
            else if constexpr (std::is_pointer_v<ElemType>)
            {
                for (size_t i = GetNum(InObject); i < InSize; ++i)
                {
                    Vector.push_back(new ElemType());
                }
            }
            else
            {
                Vector.resize(InSize);
            }
        }

        virtual void Clear(const void* InObject) override
        {
            GetVector(InObject).clear();
        }

        virtual size_t GetNum(const void* InObject) const override
        {
            return GetVector(InObject).size();
        }

        virtual void* GetKeyInstance()
        {
            return nullptr;
        }

        virtual void* GetValueInstance()
        {
            return &ValueInstance;
        }

        // --------------------------------------------------------------------------------------------
        // FVectorProperty

        virtual void Reserve(const void* InObject, size_t InCapacity)
        {
            GetVector(InObject).reserve(InCapacity);
        }

        virtual void* Get(const void* InObject, const size_t InIndex) override
        {
            auto& Vector = GetVector(InObject);
            if constexpr (is_smart_ptr_v<ElemType>)
            {
                return &Vector[InIndex];
            }
            else if constexpr (std::is_pointer_v<ElemType>)
            {
                return Vector[InIndex];
            }
            else
            {
                return &Vector[InIndex];
            }
        }

        virtual void Set(const void* InObject, const size_t InIndex, void* InData) override
        {
            auto& Vector = GetVector(InObject);
            if constexpr (is_smart_ptr_v<ElemType>)
            {
                // shared_ptr이 관리, 포인터 이동
                Vector[InIndex] = *static_cast<ElemType*>(InData);
            }
            else
            {
                ElemType* ValuePtr = static_cast<ElemType*>(InData);
                ElemType Value = *ValuePtr;

                Vector[InIndex] = Value;
            }

            if (Func)
            {
                Func((OwnerType*)InObject);
            }
        }

        virtual void PushBack(const void* InObject, void*& InData) override
        {
            auto& Vector = GetVector(InObject);

            if constexpr (is_smart_ptr_v<ElemType>)
            {
                // 복사 생성
                if (InData == nullptr)
                {
                    Vector.push_back(nullptr);
                }
                else
                {
                    ElemType Elem = *static_cast<ElemType*>(InData);
                    Vector.push_back(Elem);
                }
            }
            else if constexpr (std::is_pointer_v<ElemType>)
            {
                // 포인터 이동
                Vector.push_back(static_cast<PureType*>(InData));
                InData = nullptr;
            }
            else
            {
                ElemType* Elem = static_cast<ElemType*>(InData);
                Vector.push_back(*Elem);
            }
        }
    };

    using Type = std::conditional_t<std::is_array_v<ElemType>, std::remove_extent_t<ElemType>, ElemType>;

    std::function<void(OwnerType* InObject)> Func = InFunc;
    FVectorPropertyDesc* NewDesc = new FContainerDescImple(MemPtr, Func);
    NewDesc->Name = InName;
    NewDesc->Size = sizeof(PureType);
    NewDesc->ContainerType = EContainerType::Vector;
    NewDesc->ContainerKeyType = EType::Int;
    NewDesc->bSharedPtr = is_smart_ptr_v<ElemType>;

    SetType<PureType>(NewDesc->Type, NewDesc->TypeDesc);

    return NewDesc;
}