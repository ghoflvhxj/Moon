#pragma once
#include "Include.h"

enum class EPRopertyType
{
    None,
    Int,
    Float,
    String,
    Bool,
    Object,
    Array,
    Map
};

struct FProperty
{
    std::string Name;
    void* Data = nullptr;
    EPRopertyType Type = EPRopertyType::None;
};

class MSerializable
{
    virtual std::vector<FProperty> GetProperties() const = 0;
};

