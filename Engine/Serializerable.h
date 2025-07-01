#pragma once

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
    std::string Name = EPRopertyType::None;
    void* Property = nullptr;
    EPRopertyType Type;
};

class Serializeable
{
    std::vector<FProperty> GetProperties() const = 0;
};

