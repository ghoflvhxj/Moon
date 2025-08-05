#pragma once

#include "Include.h"
#include "Object.h"

template <class ReturnType, class... ParamTypes>
struct ENGINE_DLL FDelegate
{
public:
    struct FDelegateData
    {
        std::weak_ptr<MObject> Object;
        std::function<ReturnType(ParamTypes...)> Func;
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

    void Broadcast(ParamTypes... args)
    {
        for (auto& DelegateData : DelegateDatas)
        {
            if (std::shared_ptr<MObject> Object = DelegateData.Object.lock())
            {
                DelegateData.Func(args...);
            }
        }
    }

protected:
    std::vector<FDelegateData> DelegateDatas;
};