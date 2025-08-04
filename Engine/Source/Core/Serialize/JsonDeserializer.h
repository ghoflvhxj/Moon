#pragma once

/*
Json을 읽어 오브젝트 데이터를 채움.
리플렉션 기능을 활용해 구현되며, 현재 컨테이너는 벡터 타입만 지원하도록 구현됨.

주요 함수와 설명
Deserialize             - Json을 읽어 오브젝트 데이터를 채우는 함수.

GetObjectFromJson       -
                          컴파일 타임에 T에 따라 분기하여, 즉시 JsonValue를 만들거나 SerializeCustomType을 호출함
DeserializeCustomType   - 커스텀 타입을 Json으로 부터 읽어오는 함수.
*/


#include "Include.h"

#include "Core/FileSystem.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"

class ENGINE_DLL MJsonDeserializer
{
public:
    MJsonDeserializer();

protected:
    rapidjson::Document Doc;
    rapidjson::MemoryPoolAllocator<>& Allocator;

public:
    // 리플렉션에 등록된 클래스나 구조체를 Json으로 부터 읽어옴.
    template <class T>
    void Deserialize(T& OutObject, const std::wstring& InPath)
    {
        std::filesystem::path Path(InPath);
        if (Path.is_absolute() == false)
        {
            Path = MFIleSystem::CombinePath(Path);
        }

        if (PathFileExists(Path.c_str()) == false)
        {
            std::wstring Msg = TEXT("파일이 없음: ") + Path.wstring();
            MSGBOX(Msg);
            return;
        }

        FILE* fp = nullptr;
        _wfopen_s(&fp, Path.c_str(), TEXT("rb"));

        char readBuffer[2048];
        rapidjson::FileReadStream frs(fp, readBuffer, sizeof(readBuffer));
        Doc.ParseStream(frs);

        fclose(fp);

        const FTypeDesc* TypeDesc = OutObject.GetTypeDesc();
        while (TypeDesc)
        {
            if (Doc.HasMember(TypeDesc->Name))
            {
                PatchStruct(TypeDesc, &OutObject, Doc.FindMember(TypeDesc->Name)->value);
            }
            else
            {
                std::wstring Msg = TEXT("Document 멤버를 찾을 수 없음: ") + StringToWString(TypeDesc->Name.data());
                MSGBOX(Msg);
            }
            
            TypeDesc = TypeDesc->Parent;
        }
    }

    template <class T>
    void Deserialize(std::shared_ptr<T> OutObject, const std::wstring& Path)
    {
        Deserialize(*OutObject, Path);
    }

public:
    void PatchStruct(const FTypeDesc* InTypeDesc, void* InObject, rapidjson::Value& InValue);
    void PatchContainer(void* InContainer, FContainerPropertyDesc* InContainerPropDesc, rapidjson::Value& InJsonValue, void* InObject);

public:
    // 일반 타입 DeserializeCustomType. 실제로는 호출되지 않지만, 다른 특수화된 템플릿을 생성하기 위해 존재.
    template <class T>
    void DeserializeCustomType(rapidjson::Value& JsonValue, T& OutObject)
    {  
    }

    // wstring
    template <>
    void DeserializeCustomType(rapidjson::Value& JsonValue, std::wstring& OutString)
    {
        std::string temp = JsonValue.GetString();
        StringToWString(temp.c_str(), OutString);
    }
    
    // Vec2
    template <>
    void DeserializeCustomType(rapidjson::Value& JsonValue, Vec2& Vector)
    {
        Vector.x = JsonValue.GetArray()[0].GetFloat();
        Vector.y = JsonValue.GetArray()[1].GetFloat();
    }

    // Vec3
    template <>
    void DeserializeCustomType(rapidjson::Value& JsonValue, Vec3& Vector)
    {
        Vector.x = JsonValue.GetArray()[0].GetFloat();
        Vector.y = JsonValue.GetArray()[1].GetFloat();
        Vector.z = JsonValue.GetArray()[2].GetFloat();
    }

    // Vec4
    template <>
    void DeserializeCustomType(rapidjson::Value& JsonValue, Vec4& Vector)
    {
        Vector.x = JsonValue.GetArray()[0].GetFloat();
        Vector.y = JsonValue.GetArray()[1].GetFloat();
        Vector.z = JsonValue.GetArray()[2].GetFloat();
        Vector.w = JsonValue.GetArray()[3].GetFloat();
    }

public:
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
            DeserializeCustomType(InJsonValue, OutObject);
        }
    }

    // T*, T[N]
    template <class T>
    void GetObjectFromJson(rapidjson::Value& InJsonValue, T* OutObject, uint32 Num)
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            for (uint32 i = 0; i < Num; ++i)
            {
                OutObject[i] = InJsonValue[i].Get<T>();
            }

            // 컨테이너 버전
            //for (auto Iter = InJsonValue.MemberBegin(); Iter != InJsonValue.MemberEnd(); ++Iter)
            //{
            //    uint32 Index = std::stoi(Iter->name.GetString());
            //    OutObject[Index] = Iter->value.Get<T>();
            //}
        }
        else
        {
            for (uint32 i = 0; i < Num; ++i)
            {
                DeserializeCustomType(InJsonValue[i], OutObject[i]);
            }
        }
    }
};