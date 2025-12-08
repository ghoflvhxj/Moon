#pragma once

#include "Macro.h"
#include "TypeDesc.h"

#include "Property/FundamentaProperty.h"
#include "Property/VectorProperty.h"
#include "Property/MapProperty.h"


void ReleaseReflection();

#define PROPERTY(Prop) \
MakeProp(#Prop, &Self::Prop, nullptr)

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
	        { __VA_ARGS__ }, \
        }; \
        \
        AddTypeDesc<Self>(&MyClass##_Desc); \
        \
	    return &MyClass##_Desc; \
    } \
private: \
inline static const FTypeDesc* MyClass##_Desc = GetTypeDescStatic(); 

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
	        { __VA_ARGS__ }, \
        }; \
        \
        AddTypeDesc<Self>(&MyClass##_Desc); \
        \
	    return &MyClass##_Desc; \
    } \
    std::shared_ptr<Self> GetShared() \
    { \
        return static_pointer_cast<Self>(shared_from_this()); \
    } \
    std::shared_ptr<const Self> GetShared() const \
    { \
        return static_pointer_cast<const Self>(shared_from_this()); \
    } \
private: \
inline static const FTypeDesc* MyClass##_Desc = GetTypeDescStatic(); 

template <>
const FTypeDesc* GetTypeDesc<Vec2>();
template <>
const FTypeDesc* GetTypeDesc<Vec3>();
template <>
const FTypeDesc* GetTypeDesc<Vec4>();
template <>
const FTypeDesc* GetTypeDesc<Mat4>();
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