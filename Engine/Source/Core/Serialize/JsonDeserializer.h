#pragma once

#include "Include.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"

#include "Vertex.h"

#undef GetObject

//struct Test
//{
//    std::array<Vertex, 2> Vertices;
//    REFLECTABLE(
//        REFLECT_FIELD(Test, Vertices, EType::Array)
//    );
//};

class ENGINE_DLL MJsonDeserializer
{
public:
    MJsonDeserializer();

protected:
    rapidjson::Document Doc;
    rapidjson::MemoryPoolAllocator<>& Allocator;

public:
    template <class T>
    void Deserialize(T& OutObject, const std::wstring& Path)
    {
        FILE* fp = nullptr;
        _wfopen_s(&fp, Path.c_str(), TEXT("rb"));

        char readBuffer[2048];
        rapidjson::FileReadStream frs(fp, readBuffer, sizeof(readBuffer));
        Doc.ParseStream(frs);

        fclose(fp);

        auto Fields = T::GetFields();
        std::string name = Doc.FindMember(std::get<0>(std::get<0>(Fields)))->name.GetString();
        std::apply([&](auto& ...Field) {
            ((GetObjectFromJson(Doc.FindMember(std::get<0>(Field))->value, OutObject.*(std::get<1>(Field)))), ...);
        }, Fields);
    }

    template <class T>
    void Deserialize(std::shared_ptr<T> OutObject, const std::wstring& Path)
    {
        Deserialize(*OutObject, Path);
    }

public:
    // 구조체, 클래스
    template <class T>
    void TDeserialize(rapidjson::Value& JsonValue, T& OutObject)
    {
        auto Fields = T::GetFields();
        std::string name = JsonValue.FindMember(std::get<0>(std::get<0>(Fields)))->name.GetString();
        std::apply([&](auto& ...Field) {
            ((GetObjectFromJson(JsonValue.FindMember(std::get<0>(Field))->value, OutObject.*(std::get<1>(Field)))), ...);
        }, Fields);
    }

    // 스마트 포인터
    template <class T>
    void TDeserialize(rapidjson::Value& JsonValue, std::shared_ptr<T>& OutObject)
    {
        TDeserialize(JsonValue, *OutObject.get());
    }
    
    // 스마트 포인터 벡터
    template <class T>
    void TDeserialize(rapidjson::Value& JsonValue, std::vector<std::shared_ptr<T>>& OutVector)
    {
        if (OutVector.empty())
        {
            uint32 Num = JsonValue.MemberCount();
            for (uint32 i = 0; i < Num; ++i)
            {
                std::shared_ptr<T> NewT = std::make_shared<T>();
                TDeserialize(JsonValue.FindMember(std::to_string(i))->value, NewT);
                OutVector.push_back(NewT);
            }
        }
        else
        {
            uint32 Num = JsonValue.MemberCount();
            for (uint32 i = 0; i < Num; ++i)
            {
                OutVector[i] = std::make_shared<T>();
                TDeserialize(JsonValue.FindMember(std::to_string(i))->value, OutVector[i]);
            }
        }
    }

    // C배열
    template <class T, size_t N>
    void TDeserialize(rapidjson::Value& JsonValue, T (&OutArray)[N])
    {
        rapidjson::SizeType Num = static_cast<rapidjson::SizeType>(N);
        if (JsonValue.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < Num; ++i)
            {
                GetObjectFromJson(JsonValue.GetArray()[i], OutArray[i]);
            }
        }
        if (JsonValue.IsObject())
        {
            for (rapidjson::SizeType i = 0; i < Num; ++i)
            {
                GetObjectFromJson(JsonValue.FindMember(std::to_string(i))->value, OutArray[i]);
            }
        }
    }

    // 배열
    template <class T, size_t N>
    void TDeserialize(rapidjson::Value& JsonValue, std::array<T, N>& ObjectArray)
    {
        rapidjson::SizeType Num = static_cast<rapidjson::SizeType>(N);
        if (JsonValue.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < N; ++i)
            {
                GetObjectFromJson(JsonValue.GetArray()[i], ObjectArray[i]);
            }
        }
        if (JsonValue.IsObject())
        {
            for (rapidjson::SizeType i = 0; i < N; ++i)
            {
                GetObjectFromJson(JsonValue.FindMember(std::to_string(i))->value, ObjectArray[i]);
            }
        }
    }

    // 벡터
    template <class T>
    void TDeserialize(rapidjson::Value& JsonValue, std::vector<T>& ObjectVector)
    {
        if (JsonValue.IsArray())
        {
            rapidjson::SizeType ElementNum = JsonValue.GetArray().Size();
            ObjectVector.resize(ElementNum);

            for (rapidjson::SizeType i = 0; i < ElementNum; ++i)
            {
                GetObjectFromJson(JsonValue.GetArray()[i], ObjectVector[i]);
            }
        }
        if (JsonValue.IsObject())
        {
            rapidjson::SizeType ElementNum = JsonValue.MemberCount();
            ObjectVector.resize(ElementNum);

            for (rapidjson::SizeType i = 0; i < ElementNum; ++i)
            {
                GetObjectFromJson(JsonValue.FindMember(std::to_string(i))->value, ObjectVector[i]);
            }
        }
    }

    // 문자열
    template <>
    void TDeserialize(rapidjson::Value& JsonValue, std::wstring& OutString)
    {
        std::string temp = JsonValue.GetString();
        StringToWString(temp.c_str(), OutString);
    }
    
    // Vec2
    template <>
    void TDeserialize(rapidjson::Value& JsonValue, Vec2& Vector)
    {
        Vector.x = JsonValue.GetArray()[0].GetFloat();
        Vector.y = JsonValue.GetArray()[1].GetFloat();
    }

    // Vec3
    template <>
    void TDeserialize(rapidjson::Value& JsonValue, Vec3& Vector)
    {
        Vector.x = JsonValue.GetArray()[0].GetFloat();
        Vector.y = JsonValue.GetArray()[1].GetFloat();
        Vector.z = JsonValue.GetArray()[2].GetFloat();
    }

    // Vec4
    template <>
    void TDeserialize(rapidjson::Value& JsonValue, Vec4& Vector)
    {
        Vector.x = JsonValue.GetArray()[0].GetFloat();
        Vector.y = JsonValue.GetArray()[1].GetFloat();
        Vector.z = JsonValue.GetArray()[2].GetFloat();
        Vector.w = JsonValue.GetArray()[3].GetFloat();
    }

    template <class T>
    void GetObjectFromJson(rapidjson::Value& InJsonValue, T& OutObject)
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            // 일반 자료형
            OutObject = InJsonValue.Get<T>();
        }
        else if constexpr (std::is_enum_v<T>)
        {
            OutObject = (T)InJsonValue.GetInt();
        }
        else
        {
            // 구조체, 클래스, 컨테이너
            TDeserialize(InJsonValue, OutObject);
        }
    }

    template <class T>
    void GetObjectFromJson(rapidjson::Value& InJsonValue, std::shared_ptr<T>& OutObject)
    {
        OutObject = std::make_shared<T>();
        if constexpr (std::is_arithmetic_v<T>)
        {
            // 일반 자료형
            OutObject = InJsonValue.Get<T>();
        }
        else
        {
            // 구조체, 클래스 등
            TDeserialize(InJsonValue, OutObject);
        }
    }

    template <>
    void GetObjectFromJson(rapidjson::Value& InJsonValue, std::wstring& OutObject)
    {
        StringToWString(InJsonValue.GetString(), OutObject);
    }

    template <>
    void GetObjectFromJson(rapidjson::Value& InJsonValue, std::string& OutObject)
    {
        OutObject = InJsonValue.GetString();
    }

    //void A()
    //{
    //    const char* Path = "D:\\Git\\Moon\\Client\\test.json";

    //    Test t;
    //    MJsonDeserializer Deserializer;
    //    Deserializer.Deserialize(t, Path);
    //}
};

