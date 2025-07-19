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
virtual const FTypeDesc& GetTypeDesc() \
{ \
	return Self::GetTypeDescStatic(); \
} \
static const FTypeDesc& GetTypeDescStatic() \
{ \
	static FTypeDesc MyClass##_Desc = { \
    nullptr, \
	#MyClass, \
	sizeof(MyClass), \
	{ __VA_ARGS__ }}; \
	return MyClass##_Desc; \
}

#define REFLECT(MyClass, ...) \
public: \
using Super = Self; \
using Self = MyClass; \
virtual const FTypeDesc& GetTypeDesc() \
{ \
	return Self::GetTypeDescStatic(); \
} \
static const FTypeDesc& GetTypeDescStatic() \
{ \
	static FTypeDesc MyClass##_Desc = { \
	&Super::GetTypeDescStatic(), \
	#MyClass, \
	sizeof(MyClass), \
	{ __VA_ARGS__ }}; \
	return MyClass##_Desc; \
}
