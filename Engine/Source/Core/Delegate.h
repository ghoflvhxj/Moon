#pragma once

#include "Include.h"
#include "Object.h"

template <class ReturnType, class... ParamTypes>
struct ENGINE_DLL FDelegate
{
public:
    FDelegate() = default;
    FDelegate(const FDelegate&) = delete;
    FDelegate& operator=(const FDelegate&) = delete;

public:
    struct FDelegateData
    {
        std::weak_ptr<MObject> Object;
        std::function<ReturnType(ParamTypes...)> Func;
    };
    struct FRawDelegateData
    {
        virtual ReturnType Execute(ParamTypes... args) {}
    };
    template <class T>
    struct FRawDelegateDataProxy : public FRawDelegateData
    {
        T* Object;
        ReturnType (T::*Func)(ParamTypes...);

        virtual ReturnType Execute(ParamTypes... args) override
        {
            return (Object->*Func)(args...);
        }
    };

public:
    template <class T>
    void Add(std::shared_ptr<T> InObject, std::function<ReturnType(ParamTypes...)> InFunc)
    {
        FDelegateData NewData = {
            InObject,
            InFunc
        };

        DelegateDatas.push_back(NewData);
    }

    void Add(std::function<ReturnType(ParamTypes...)> InFunc)
    {
        Lambdas.push_back(InFunc);
    }

    template <class T>
    void Add(std::shared_ptr<T> InObject, ReturnType(T::* InFunc)(ParamTypes...))
    {
        Add(
            InObject,
            [&](ParamTypes... args)->ReturnType {
                return (InObject.get()->*InFunc)(args...);
            }
        );
    }

    template <class T>
    void Add(T* InObject, ReturnType(T::* InFunc)(ParamTypes...))
    {
        auto NewData = std::make_unique<FRawDelegateDataProxy<T>>();
        NewData->Object = InObject;
        NewData->Func = InFunc;

        RawDelegateDatas.push_back(std::move(NewData));
    }

    void Broadcast(ParamTypes... args)
    {
        for (auto& DelegateData : DelegateDatas)
        {
            if (std::shared_ptr<MObject> Object = DelegateData.Object.lock())
            {
                DelegateData.Func(args...);
            }
        }

        for (auto& DelegateData : RawDelegateDatas)
        {
            DelegateData->Execute(args...);
        }

        for (auto& Lambda : Lambdas)
        {
            Lambda(args...);
        }
    }

protected:
    std::vector<FDelegateData> DelegateDatas;
    std::vector<std::unique_ptr<FRawDelegateData>> RawDelegateDatas;
    std::vector<std::function<ReturnType(ParamTypes...)>> Lambdas;
};