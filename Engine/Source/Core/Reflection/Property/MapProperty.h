#pragma once

#include "ContainerProperty.h"
#include <map>
#include <unordered_map>

struct FMapPropertyDesc : public FPropertyDesc, public FContainerPropertyInterface
{
    virtual void* Get(const void* InObject, const void* InKey) = 0;
    virtual void Set(const void* InObject, void* &InKey, void* &InData) = 0;
    virtual std::vector<const void*> GetKeys(const void* InObject) { return std::vector<const void*>(); }
};

template <class Owner, class KeyType, class ElemType, class F >
static FPropertyDesc* MakeProp(const std::string& InName, std::unordered_map<KeyType, ElemType> Owner::* MemPtr, F InFunc)
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
        FContainerDescImple(std::unordered_map<KeyType, ElemType> Owner::* MemPtr, std::function<void(Owner* InObject)> InFunc)
            : MemPtr(MemPtr), Func(InFunc)
        {
        }
        std::unordered_map<KeyType, ElemType> Owner::* MemPtr = nullptr;
        std::function<void(Owner* InObject)> Func;

        inline std::unordered_map<KeyType, ElemType>& GetMap(const void* InObject) const
        {
            return (Owner*)InObject->*MemPtr;
        }

        // FPropertyDesc Iterface ----------------------------------------------------
        virtual void* GetAsVoid(const void* InObject, size_t InIndex = 0) override
        {
            // DoNothing
            return nullptr;
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

        virtual void Set(const void* InObject, void*& InKey, void*& InData) override
        {
            KeyType Key; 
            if constexpr (std::is_pointer_v<KeyType>)
            {
                KeyType Data = static_cast<KeyType>(InKey);
            }
            else
            {
                KeyType* KeyPtr = static_cast<KeyType*>(InKey);
                Key = *KeyPtr;
                delete KeyPtr;
                InKey = nullptr;
            }

            ElemType Value;
            if constexpr (std::is_pointer_v<ElemType>)
            {
                Value = static_cast<ElemType>(InData);
            }
            else
            {
                ElemType* ValuePtr = static_cast<ElemType*>(InData);
                Value = *ValuePtr;

                if (bSharedValue == false)
                {
                    delete ValuePtr;
                    InData = nullptr;
                }
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

    std::function<void(Owner* InObject)> Func = InFunc;
    FMapPropertyDesc* NewDesc = new FContainerDescImple(MemPtr, Func);
    NewDesc->Name = InName;
    NewDesc->Size = sizeof(PureType);
    NewDesc->ContainerType = EContainerType::Unordered_map;
    NewDesc->bSharedValue = is_smart_ptr_v<ElemType>;

    SetType<KeyType>(NewDesc->ContainerKeyType, NewDesc->KeyTypeDesc);
    SetType<PureType>(NewDesc->Type, NewDesc->TypeDesc);

    return NewDesc;
}
