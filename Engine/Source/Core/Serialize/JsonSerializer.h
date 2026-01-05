#pragma once

/*
클래스, 구조체 같은 커스텀 자료형을 Json으로 만듬.
리플렉션 기능을 활용해 구현되며, 현재 컨테이너는 벡터 타입만 지원하도록 구현됨.

주요 함수와 설명

Serialize               - 사용자가 호출할 함수. 내부적으로 Serialize 로직이 실행됨.

DispatchStruct          - 실질적으로 클래스, 구조체 같은 커스텀 자료형을 Json으로 만드는 함수

DispatchContainer       - 컨테이너를 Json으로 만드는 함수

ToJsonValue             - T, T*, T[N], vector<T> 를 받아 JsonValue로 만드는 함수. 
                          컴파일 타임에 T에 따라 분기하여, 즉시 JsonValue를 만들거나 SerializeCustomType을 호출함

SerializeCustomType     - 커스텀 타입을 JsonValue로 만들어서 반환하는 함수.
*/

#include "Include.h"

#include "Core/FileSystem.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"

class MSerializable;

class ENGINE_DLL MJsonSerializer
{
    rapidjson::Value HandleData(EType InType, const void* InData);
    rapidjson::Value HandleData(const FTypeDesc* InTypeDesc, const void* InData, bool bShared = false);
    template <class T>
    T CastData(const void* InData)
    {
        return *static_cast<const T*>(InData);
    }

public:
    MJsonSerializer();
protected:
    rapidjson::Document Doc;
    rapidjson::MemoryPoolAllocator<>& Allocator;

public:
    rapidjson::Value DispatchStruct(const FTypeDesc* InTypeDesc, const void* InData);
    rapidjson::Value DispatchVector(FVectorPropertyDesc* InContainerPropDesc, const void* InData);
    rapidjson::Value DispatchMap(FMapPropertyDesc* InContainerPropDesc, const void* InData);
    rapidjson::Value DispatchArray(FPropertyDesc* InArrayPropDesc, const void* InData);


public:
    // 리플렉션에 등록된 클래스나 구조체를 Json으로 만듬.
    template <class T>
    void Serialize(T& Object, const std::wstring& InPath, bool bPretty)
    {
        if(InPath.empty())
        {
            std::wstring Msg = TEXT("잘못된 경로: ") + InPath;
            MSGBOX(Msg);
            return;
        }

        const FTypeDesc* Current = Object.GetTypeDesc();
        while (Current)
        {
            Doc.AddMember(ToJsonValue(Current->Name), DispatchStruct(Current, &Object), Allocator);
            Current = Current->Parent;
        }

        std::wstring Path = MFileSystem::AbsolutePath(InPath);

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

    // 일반 포인터 타입 지원을 위한 오버로딩
    template <class T>
    void Serialize(T* InObject, const std::wstring& InPath, bool bInPretty)
    {
        Serialize(*InObject, InPath, bInPretty);
    }

    // shared_ptr을 지원을 위한 오버로딩
    template <class T>
    void Serialize(std::shared_ptr<T> InObject, const std::wstring& InPath, bool bInPretty)
    {
        Serialize(*InObject, InPath, bInPretty);
    }



public:
    // 일반 타입 SerializeCustomType. 실제로는 호출되지 않지만, 다른 특수화된 템플릿을 생성하기 위해 존재.
    template <class T>
    rapidjson::Value SerializeCustomType(const T& Object)
    {
        rapidjson::Value OutValue(rapidjson::kObjectType);
        return OutValue;
    }

    // string
    template <>
    rapidjson::Value SerializeCustomType(const std::string& Object)
    {
        return rapidjson::Value(Object.c_str(), Allocator);
    }

    // wstring
    template <>
    rapidjson::Value SerializeCustomType(const std::wstring& Object)
    {
        char Buffer[256];
        WStringToString(Object, Buffer, sizeof(Buffer));
        return rapidjson::Value(Buffer, Allocator);
    }

    // Vec2
    template <>
    rapidjson::Value SerializeCustomType(const Vec2& Object)
    {
        rapidjson::Value ArrayValue(rapidjson::kArrayType);
        ArrayValue.PushBack(rapidjson::Value(Object.x), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.y), Allocator);

        return ArrayValue;
    }

    // Vec3
    template <>
    rapidjson::Value SerializeCustomType(const Vec3& Object)
    {
        rapidjson::Value ArrayValue(rapidjson::kArrayType);
        ArrayValue.PushBack(rapidjson::Value(Object.x), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.y), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.z), Allocator);

        return ArrayValue;
    }

    // Vec4
    template <>
    rapidjson::Value SerializeCustomType(const Vec4& Object)
    {
        rapidjson::Value ArrayValue(rapidjson::kArrayType);
        ArrayValue.PushBack(rapidjson::Value(Object.x), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.y), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.z), Allocator);
        ArrayValue.PushBack(rapidjson::Value(Object.w), Allocator);

        return ArrayValue;
    }

    // Mat4
    template <>
    rapidjson::Value SerializeCustomType(const Mat4& InData)
    {
        //rapidjson::Value ArrayValue(rapidjson::kArrayType);

        //ArrayValue.PushBack(ToJsonValue(InData.m), Allocator);
        //return ArrayValue;

        return ToJsonValue(InData.m);
    }

public:
    // ToJson T
    template <class T>
    rapidjson::Value ToJsonValue(const T& InValue)
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            return rapidjson::Value(InValue);
        }
        else
        {
            return SerializeCustomType(InValue);
        }
    }

    // ToJson T*
    template <class T>
    rapidjson::Value ToJsonValue(const T* InValue, size_t Num, bool bContainer = false)
    {
        if (bContainer)
        {
            rapidjson::Value ObjectValue(rapidjson::kObjectType);
            if constexpr (std::is_arithmetic_v<T>)
            {
                for (size_t i = 0; i < Num; ++i)
                {
                    ObjectValue.AddMember(rapidjson::Value(std::to_string(i), Allocator), rapidjson::Value(InValue[i]), Allocator);
                }
            }
            else
            {
                for (size_t i = 0; i < Num; ++i)
                {
                    ObjectValue.AddMember(rapidjson::Value(std::to_string(i), Allocator), SerializeCustomType(InValue[i]), Allocator);
                }
            }
            return ObjectValue;
        }
        else
        {
            rapidjson::Value ArrayValue(rapidjson::kArrayType);
            if constexpr (std::is_arithmetic_v<T>)
            {
                for (size_t i = 0; i < Num; ++i)
                {
                    ArrayValue.PushBack(rapidjson::Value(InValue[i]), Allocator);
                }
            }
            else
            {
                for (size_t i = 0; i < Num; ++i)
                {
                    ArrayValue.PushBack(SerializeCustomType(InValue[i]), Allocator);
                }
            }
            return ArrayValue;
        }
    }

    // ToJson T[N]
    template <class T, size_t N>
    rapidjson::Value ToJsonValue(T (&InValue)[N])
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            rapidjson::Value ArrayValue(rapidjson::kArrayType);
            for (size_t i = 0; i < N; ++i)
            {
                ArrayValue.PushBack(rapidjson::Value(InValue[i]), Allocator);
            }
            return ArrayValue;
        }
        else
        {
            return SerializeCustomType(InValue);
        }
    }

    template <class T, size_t N, size_t M>
    rapidjson::Value ToJsonValue(T(&InValue)[N][M])
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            rapidjson::Value ArrayValue(rapidjson::kArrayType);
            for (size_t i = 0; i < N; ++i)
            {
                for (size_t j = 0; j < M; ++j)
                {
                    ArrayValue.PushBack(rapidjson::Value(InValue[i][j]), Allocator);
                }
            }

            return ArrayValue;
        }
        else
        {
            return SerializeCustomType(InValue);
        }
    }

    // ToJson vector<T>
    template <class T>
    rapidjson::Value ToJsonValue(const std::vector<T>& InValue)
    {
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
        else // 구조체, 클래스 등
        { 
            return SerializeCustomType(InValue);
        }
    }
};


