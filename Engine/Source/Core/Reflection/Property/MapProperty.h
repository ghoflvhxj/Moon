#pragma once

#include "ContainerProperty.h"
#include <any>

struct FMapPropertyDesc : public FPropertyDesc, public FContainerPropertyInterface
{
    virtual void* Get(const void* InObject, const void* InKey) = 0;
    virtual void Set(const void* InObject, const void* InKey, const void* InData) = 0;
    virtual std::vector<const void*> GetKeys(const void* InObject) { return std::vector<const void*>(); }
};

template <class OwnerType, class KeyType, class ElemType, class F >
static FPropertyDesc* MakeProp(const std::string& InName, std::unordered_map<KeyType, ElemType> OwnerType::* MemPtr, F InFunc)
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

    struct FContainerDescImple : public FMapPropertyDesc
    {
        FContainerDescImple(std::unordered_map<KeyType, ElemType> OwnerType::* MemPtr, std::function<void(OwnerType* InObject)> InFunc)
            : MemPtr(MemPtr), Func(InFunc)
        {
        }
        std::unordered_map<KeyType, ElemType> OwnerType::* MemPtr = nullptr;
        std::function<void(OwnerType* InObject)> Func;

        KeyType KeyInstance;
        ElemType ValueInstance;

        inline std::unordered_map<KeyType, ElemType>& GetMap(const void* InObject) const
        {
            return (OwnerType*)InObject->*MemPtr;
        }

        // FPropertyDesc Interface ----------------------------------------------------
        virtual void* GetAsVoid(const void* InObject, size_t InIndex = 0) override
        {
            // DoNothing
            return nullptr;
        }

        virtual void Copy(const void* InSrcObject, void* InDstObject, size_t InIndex = 0) override
        {
            const auto& SrcMap = GetMap(InSrcObject);
            auto& DstMap = GetMap(InDstObject);

            DstMap.clear();

            for (auto& [Key, Value] : SrcMap)
            {
                const void* SrcKeyPtr = &Key;
                const void* SrcValuePtr = &Value;
                Set(InDstObject, SrcKeyPtr, SrcValuePtr);
            }
        }

        // FContainerProperty Interface ----------------------------------------------------
        virtual void Resize(const void* InObject, const size_t InSize) override
        {
            // DoNothing
        }

        virtual size_t GetNum(const void* InObject) const override
        {
            return GetMap(InObject).size();
        }

        virtual void Clear(const void* InObject) override
        {
            GetMap(InObject).clear();
        }

        virtual void* GetKeyInstance()
        {
            return &KeyInstance;
        }

        virtual void* GetInstance()
        {
            return &ValueInstance;
        }

        // FMapProperty Interface ----------------------------------------------------
        virtual void* Get(const void* InObject, const void* InKey) override
        {
            const KeyType& Key = *static_cast<const KeyType*>(InKey);
            if (GetMap(InObject).find(Key) == GetMap(InObject).end())
            {
                return nullptr;
            }

            if constexpr (is_smart_ptr_v<ElemType>)
            {
                return &GetMap(InObject)[Key];
            }
            else if constexpr (std::is_pointer_v<ElemType>)
            {
                return GetMap(InObject)[Key];
            }
            else
            {
                return &GetMap(InObject)[Key];
            }
        }

        virtual void Set(const void* InObject, const void* InKey, const void* InData) override
        {
            KeyType Key; 
            if constexpr (std::is_pointer_v<KeyType>)
            {
                // TODO. 언젠가는 지원해야 함
            }
            else
            {
                Key = *static_cast<const KeyType*>(InKey);
            }

            ElemType Value;
            if constexpr (std::is_pointer_v<ElemType>)
            {
                // TODO. 언젠가는 지원해야 함
            }
            else
            {
                Value = *static_cast<const ElemType*>(InData);
            }
            
            GetMap(InObject)[Key] = Value;
        }

        virtual std::vector<const void*> GetKeys(const void* InObject) override
        {
            std::unordered_map<KeyType, ElemType>& Map = GetMap(InObject);
            std::vector<const void*> Keys;

            for (auto& [Key, Value] : GetMap(InObject))
            {
                Keys.push_back(&Key);
            }

            return Keys;
        }
    };

    using Type = std::conditional_t<std::is_array_v<ElemType>, std::remove_extent_t<ElemType>, ElemType>;

    std::function<void(OwnerType* InObject)> Func = InFunc;
    FMapPropertyDesc* NewDesc = new FContainerDescImple(MemPtr, Func);
    NewDesc->Name = InName;
    NewDesc->Size = sizeof(PureType);
    NewDesc->ContainerType = EContainerType::Unordered_map;
    NewDesc->bSharedPtr = is_smart_ptr_v<ElemType>;

    SetType<KeyType>(NewDesc->ContainerKeyType, NewDesc->KeyTypeDesc);
    SetType<PureType>(NewDesc->Type, NewDesc->TypeDesc);

    return NewDesc;
}
