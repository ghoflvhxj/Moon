#include "Reflection.h"
#include "Function.h"

void ReleaseReflection()
{
    std::wstring Msg = TEXT("Release Reflection");
    LOG(Msg);

    for (auto& [TypeDesc, Factory] : GetFactory())
    {
        delete Factory;
    }

    for (auto& [Name, TypeDesc] : GetTypeDescs())
    {
        std::wstring str = StringToWString(TypeDesc->Name) + TEXT(" Release");
        LOG(str);

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

template <>
const FTypeDesc* GetTypeDesc<Mat4>()
{
    static FTypeDesc TypeDesc = {
        nullptr,
        "Mat4",
        sizeof(Mat4),
        {
            MakeProp("m", &Mat4::m, nullptr)
        }
    };

    return &TypeDesc;
}