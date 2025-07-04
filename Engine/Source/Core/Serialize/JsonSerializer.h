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

    // 시리얼 라이즈 할 때 사용하는 함수
    // 클래스나 구조체 타입만을 받음. 즉, vector, map, int 등은 못받음
    template <class T>
    void Serialize(const T& Object, const std::wstring& Path, bool bPretty)
    {
        constexpr auto Fields = T::GetFields();
        std::apply([&, Object](auto&& ...Field) {
            ((Doc.AddMember(rapidjson::Value(std::get<0>(Field), Allocator), ToJsonValue(Object.*(std::get<1>(Field))), Allocator)), ...);
        }, Fields);

        FILE* fp = nullptr;
        _wfopen_s(&fp, Path.c_str(), TEXT("wb"));

        char WriteBuffer[4096];
        rapidjson::FileWriteStream WriteStream(fp, WriteBuffer, sizeof(WriteBuffer));

        if (bPretty)
        {
            rapidjson::PrettyWriter<rapidjson::FileWriteStream> JsonWriter(WriteStream);
            JsonWriter.SetMaxDecimalPlaces(5);
            Doc.Accept(JsonWriter);
        }
        else
        {
            rapidjson::Writer<rapidjson::FileWriteStream> JsonWriter(WriteStream);
            JsonWriter.SetMaxDecimalPlaces(5);
            Doc.Accept(JsonWriter);
        }

        fclose(fp);
    }

    // 시리얼 라이즈 할 때 사용하는 함수.
    // 클래스나 구조체의 shared_ptr를 받음
    template <class T>
    void Serialize(std::shared_ptr<T>& Object, const std::wstring& Path, bool bPretty)
    {
        Serialize(*Object, Path, bPretty);
    }

public:
    // 오브젝트
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

    // shared_ptr
    template <class T>
    rapidjson::Value TSerialize(const std::shared_ptr<T>& Object)
    {
        return TSerialize(*Object.get());
    }

    // 배열
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

    template <class T, size_t N>
    rapidjson::Value TSerialize(const std::array<T, N> Objects)
    {
        rapidjson::Value OutValue(rapidjson::kObjectType);

        constexpr auto Fields = T::GetFields();
        for (size_t i = 0; i < N; ++i)
        {
            rapidjson::Value ElemValue(rapidjson::kObjectType);

            // Element를 Serialize
            const T& Object = Objects[i];
            std::apply([&, Object](auto&& ...Field) {
                ((ElemValue.AddMember(rapidjson::Value(std::get<0>(Field), Allocator), ToJsonValue(Object.*(std::get<1>(Field))), Allocator)), ...);
                }, Fields);

            // Serialize된 Element 추가
            OutValue.AddMember(rapidjson::Value(std::to_string(i), Allocator), ElemValue, Allocator);
        }

        return OutValue;
    }

    // 벡터
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

    // shared_ptr 배열
    template <class T>
    rapidjson::Value TSerialize(const std::vector<std::shared_ptr<T>>& Objects)
    {
        std::vector<T> Vector;
        Vector.reserve(Objects.size());
        for (auto& Object : Objects)
        {
            if (Object)
            {
                Vector.push_back(*Object.get());
            }
            else
            {
                T Dummy;
                Vector.push_back(Dummy);
            }
        }
        return TSerialize(Vector);
    }

    // 문자열
    template <>
    rapidjson::Value TSerialize(const std::wstring& Object)
    {
        char Buffer[256];
        WStringToString(Object, Buffer, sizeof(Buffer));
        return rapidjson::Value(Buffer, Allocator);
    }

    // Vec2
    template <>
    rapidjson::Value TSerialize(const Vec2& Object)
    {
        rapidjson::Value ArrayValue(rapidjson::kArrayType);
        ArrayValue.PushBack(rapidjson::Value(Object.x), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.y), Allocator);

        return ArrayValue;
    }

    // Vec3
    template <>
    rapidjson::Value TSerialize(const Vec3& Object)
    {
        rapidjson::Value ArrayValue(rapidjson::kArrayType);
        ArrayValue.PushBack(rapidjson::Value(Object.x), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.y), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.z), Allocator);

        return ArrayValue;
    }

    // Vec4
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
        else if constexpr (std::is_enum_v<T>)
        {
            return rapidjson::Value((int)InValue);
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
        // 일반 자료형
        if constexpr (std::is_arithmetic_v<T>)
        {
            rapidjson::Value ArrayValue(rapidjson::kArrayType);
            size_t N = InValue.size();
            for (size_t i = 0; i < N; ++i)
            {
                ArrayValue.PushBack(rapidjson::Value(InValue[i]), Allocator);
            }
            return ArrayValue;
        }
        else if constexpr (std::is_enum_v<T>)
        {
            rapidjson::Value ArrayValue(rapidjson::kArrayType);
            size_t N = InValue.size();
            for (size_t i = 0; i < N; ++i)
            {
                ArrayValue.PushBack(rapidjson::Value((int)InValue[i]), Allocator);
            }
            return ArrayValue;
        }
        else // 구조체, 클래스 등
        { 
            
            return TSerialize(InValue);
        }
    }

    // vector<wstring> 특수화
    template <>
    rapidjson::Value ToJsonValue(const std::vector<std::wstring>& InValue)
    {
        // 일반 자료형
        rapidjson::Value ArrayValue(rapidjson::kArrayType);
        size_t N = InValue.size();
        for (size_t i = 0; i < N; ++i)
        {
            char Buffer[512] = {};
            WStringToString(InValue[i], Buffer, 512);
            ArrayValue.PushBack(rapidjson::Value(Buffer, Allocator), Allocator);
        }
        return ArrayValue;
    }
};


