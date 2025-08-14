#include "Reflection.h"

void ReleaseReflection()
{
    for (auto TypeDesc : GetTypeDescs())
    {
        std::string str = TypeDesc->Name + " Release";
        OutputDebugStringA(str.c_str());

        for (FPropertyDesc* Prop : TypeDesc->Properties)
        {
            delete Prop;
        }
    }
}

template <>
const FTypeDesc* GetTypeDesc<Vec2>()
{
    static FTypeDesc NewTypeDesc = {
        nullptr,
        "Vec2",
        sizeof(Vec2),
        {
            MakeProp("x", &Vec2::x, nullptr),
            MakeProp("y", &Vec2::y, nullptr)
        }
    };

    return &NewTypeDesc;
}

template <>
const FTypeDesc* GetTypeDesc<Vec3>()
{
    static FTypeDesc NewTypeDesc = {
        nullptr,
        "Vec3",
        sizeof(Vec3),
        {
            MakeProp("x", &Vec3::x, nullptr),
            MakeProp("y", &Vec3::y, nullptr),
            MakeProp("z", &Vec3::z, nullptr)
        }
    };

    return &NewTypeDesc;
}

template <>
const FTypeDesc* GetTypeDesc<Vec4>()
{
    static FTypeDesc NewTypeDesc = {
        nullptr,
        "Vec4",
        sizeof(Vec4),
        {
            MakeProp("x", &Vec4::x, nullptr),
            MakeProp("y", &Vec4::y, nullptr),
            MakeProp("z", &Vec4::z, nullptr),
            MakeProp("w", &Vec4::w, nullptr)
        }
    };

    return &NewTypeDesc;
}
