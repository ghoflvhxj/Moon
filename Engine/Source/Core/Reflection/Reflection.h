#pragma once

#include "Property/FundamentaProperty.h"
#include "Property/ContainerProperty.h"
#include "TypeDesc.h"

#define PROPERTY(Prop) \
MakeProp(#Prop, &Self::Prop, std::function<void(Self* InObject)>())

#define PROPERTY_DELEGATE(Prop, Func) \
MakeProp(#Prop, &Self::Prop, Func)

#define REFLECT_TOP(MyClass, ...) \
public: \
using Super = void; \
using Self = MyClass; \
virtual const FTypeDesc* GetTypeDesc() const \
{ \
	return Self::GetTypeDescStatic(); \
} \
static const FTypeDesc* GetTypeDescStatic() \
{ \
	static FTypeDesc MyClass##_Desc = { \
    nullptr, \
	#MyClass, \
	sizeof(MyClass), \
	{ __VA_ARGS__ }}; \
	return &MyClass##_Desc; \
}

#define REFLECT(MyClass, ...) \
public: \
using Super = Self; \
using Self = MyClass; \
virtual const FTypeDesc* GetTypeDesc() const \
{ \
	return Self::GetTypeDescStatic(); \
} \
static const FTypeDesc* GetTypeDescStatic() \
{ \
	static FTypeDesc MyClass##_Desc = { \
	Super::GetTypeDescStatic(), \
	#MyClass, \
	sizeof(MyClass), \
	{ __VA_ARGS__ }}; \
	return &MyClass##_Desc; \
} \
std::shared_ptr<Self> GetShared() \
{ \
    return static_pointer_cast<Self>(shared_from_this()); \
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
}

/*
template <>
FTypeDesc* GetTypeDesc<std::wstring>()
{
    static FTypeDesc NewTypeDesc = {
        nullptr,
        "wstring",
        0,
        {
        }
    };
}
*/