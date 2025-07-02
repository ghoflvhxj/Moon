#pragma once

#include "Include.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"

class MSerializable;
struct FProperty;

class ENGINE_DLL MJsonSerializer
{
public:
    MJsonSerializer();
protected:
    rapidjson::Document Doc;
    rapidjson::MemoryPoolAllocator<>& Allocator;

public:
    template <class T>
    rapidjson::Value TSerialize(const T& Object)
    {
        rapidjson::Value OutValue(rapidjson::kObjectType);

        constexpr auto Fields = T::GetFields();
        std::apply([&, Object](auto&& ...Field) {
            ((OutValue.AddMember(rapidjson::Value(std::get<0>(Field), Allocator), ToJsonValue(Object.*(std::get<1>(Field))), Allocator)), ...);
        }, Fields);

        return OutValue;
    }

    template <class T>
    rapidjson::Value TSerialize(const std::shared_ptr<T>& Object)
    {
        return TSerialize(*Object.get());
    }

    template <class T>
    rapidjson::Value TSerialize(const std::vector<T>& Objects)
    {
        rapidjson::Value OutValue(rapidjson::kObjectType);

        constexpr auto Fields = T::GetFields();
        size_t N = Objects.size();
        for (size_t i = 0; i < N; ++i)
        {
            rapidjson::Value ElemValue(rapidjson::kObjectType);
            const T& Object = Objects[i];

            // Element를 Serialize
            std::apply([&, Object](auto&& ...Field) {
                ((ElemValue.AddMember(rapidjson::Value(std::get<0>(Field), Allocator), ToJsonValue(Object.*(std::get<1>(Field))), Allocator)), ...);
                }, Fields);

            // Serialize된 Element 추가
            OutValue.AddMember(rapidjson::Value(std::to_string(i), Allocator), ElemValue, Allocator);
        }

        return OutValue;
    }

    template <class T>
    rapidjson::Value TSerialize(const std::vector<std::shared_ptr<T>>& Objects)
    {
        std::vector<T> Temp;
        Temp.reserve(Objects.size());
        for (auto& Object : Objects)
        {
            Temp.push_back(*Object.get());
        }
        return TSerialize(Temp);
    }

    template <class T, size_t N>
    rapidjson::Value TSerialize(T(&Objects)[N])
    {
        rapidjson::Value OutValue(rapidjson::kObjectType);

        constexpr auto Fields = T::GetFields();
        for (size_t i = 0; i < N; ++i)
        {
            rapidjson::Value ElemValue(rapidjson::kObjectType);

            // Element를 Serialize
            T& Object = Objects[i];
            std::apply([&, Object](auto&& ...Field) {
                ((ElemValue.AddMember(rapidjson::Value(std::get<0>(Field), Allocator), ToJsonValue(Object.*(std::get<1>(Field))), Allocator)), ...);
                }, Fields);

            // Serialize된 Element 추가
            OutValue.AddMember(rapidjson::Value(std::to_string(i), Allocator), ElemValue, Allocator);
        }

        return OutValue;
    }

    template <>
    rapidjson::Value TSerialize(const std::wstring& Object)
    {
        char Buffer[256];
        WStringToString(Object, Buffer, sizeof(Buffer));
        return rapidjson::Value(Buffer, Allocator);
    }

    template <>
    rapidjson::Value TSerialize(const Vec2& Object)
    {
        rapidjson::Value ArrayValue(rapidjson::kArrayType);
        ArrayValue.PushBack(rapidjson::Value(Object.x), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.y), Allocator);

        return ArrayValue;
    }

    template <>
    rapidjson::Value TSerialize(const Vec3& Object)
    {
        rapidjson::Value ArrayValue(rapidjson::kArrayType);
        ArrayValue.PushBack(rapidjson::Value(Object.x), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.y), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.z), Allocator);

        return ArrayValue;
    }

    template <>
    rapidjson::Value TSerialize(const Vec4& Object)
    {
        rapidjson::Value ArrayValue(rapidjson::kArrayType);
        ArrayValue.PushBack(rapidjson::Value(Object.x), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.y), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.z), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.w), Allocator);

        return ArrayValue;
    }

    template <class T>
    rapidjson::Value ToJsonValue(const T& InValue)
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            // 일반 자료형
            return rapidjson::Value(InValue);
        }
        else
        {
            // 구조체, 클래스 등
            return TSerialize(InValue);
        }
    }

    template <class T, size_t N>
    rapidjson::Value ToJsonValue(T (&InValue)[N])
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            // 일반 자료형
            rapidjson::Value ArrayValue(rapidjson::kArrayType);
            for (size_t i = 0; i < N; ++i)
            {
                //TSerialize(ArrayValue, InArray[i]);
                ArrayValue.PushBack(rapidjson::Value(InValue[i]), Allocator);
            }
            return ArrayValue;
        }
        else
        {
            // 구조체, 클래스 등
            return TSerialize(InValue);
        }
    }

    template <class T>
    rapidjson::Value ToJsonValue(const std::vector<T>& InValue)
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            rapidjson::Value ArrayValue(rapidjson::kArrayType);
            size_t N = InValue.size();
            for (size_t i = 0; i < N; ++i)
            {
                //TSerialize(ArrayValue, InArray[i]);
                ArrayValue.PushBack(rapidjson::Value(InValue[i]), Allocator);
            }
            return ArrayValue;
        }
        else
        {
            // 구조체, 클래스 등
            return TSerialize(InValue);
        }
    }
};


