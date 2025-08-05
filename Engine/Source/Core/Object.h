#pragma once

#include "Include.h"

class ENGINE_DLL MObject : public std::enable_shared_from_this<MObject>
{
public:
    template <class T>
    bool IsA()
    {
        const FTypeDesc* Target = T::GetTypeDescStatic();
        const FTypeDesc* Current = GetTypeDesc();
        while (Current)
        {
            if (Current == Target)
            {
                return true;
            }

            Current = Current->Parent;
        }

        return false;
    }

    template <class T>
    std::shared_ptr<T> CastTo()
    {
        return IsA<T>() ? std::static_pointer_cast<T>(shared_from_this()) : nullptr;
    }

    REFLECT_TOP(MObject)
};