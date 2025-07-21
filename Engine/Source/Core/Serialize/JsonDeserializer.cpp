#include "JsonDeserializer.h"

using namespace rapidjson;

MJsonDeserializer::MJsonDeserializer()
    : Allocator(Doc.GetAllocator())
{
}

void MJsonDeserializer::PatchStruct(const FTypeDesc* InTypeDesc, void* InObject, rapidjson::Value& InValue)
{
    if (InTypeDesc == nullptr)
    {
        return;
    }

    for (auto& Prop : InTypeDesc->Properties)
    {
        if (Prop->bContainer)   // 컨테이너
        {
            PatchContainer(Prop->GetAsVoid(InObject), static_cast<FContainerPropertyDesc*>(Prop), InValue.FindMember(Prop->Name)->value, InObject);
        }
        else if (Prop->Num > 1) // 배열
        {
            switch (Prop->Type)
            {
            case EType::Int:
            case EType::Enum:
            {
                int* Value = static_cast<FFundamentalPropertyDesc<int*>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value, Prop->Num);
            }
            break;
            case EType::Float:
            {
                float* Value = static_cast<FFundamentalPropertyDesc<float*>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value, Prop->Num);
            }
            break;
            case EType::Bool:
            {
                bool* Value = static_cast<FFundamentalPropertyDesc<bool*>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value, Prop->Num);
            }
            break;
            case EType::String:
            {
                std::string* Value = static_cast<FFundamentalPropertyDesc<std::string*>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value, Prop->Num);
            }
            break;
            case EType::WString:
            {
                std::wstring* Value = static_cast<FFundamentalPropertyDesc<std::wstring*>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value, Prop->Num);
            }
            break;
            case EType::Vec2:
            {
                Vec2* Value = static_cast<FFundamentalPropertyDesc<Vec2*>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value, Prop->Num);
            }
            break;
            case EType::Vec3:
            {
                Vec3* Value = static_cast<FFundamentalPropertyDesc<Vec3*>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value, Prop->Num);
            }
            break;
            case EType::Vec4:
            {
                Vec4* Value = static_cast<FFundamentalPropertyDesc<Vec4*>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value, Prop->Num);
            }
            break;
            default:
            {
                rapidjson::Value& Temp = InValue.FindMember(Prop->Name)->value;
                for (uint32 i = 0; i < Prop->Num; ++i)
                {
                    uint64 Base = (uint64)Prop->GetAsVoid(InObject);
                    uint64 MemoryPos = Base + (Prop->GetSize() * i);
                    PatchStruct(Prop->TypeDesc, (void*)MemoryPos, Temp[i]);
                }
            }
            break;
            }
        }
        else // 단일 
        {
            switch (Prop->Type)
            {
            case EType::Int:
            case EType::Enum:
            {
                int& Value = static_cast<FFundamentalPropertyDesc<int>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value);
            }
            break;
            case EType::Float:
            {
                float& Value = static_cast<FFundamentalPropertyDesc<float>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value);
            }
            break;
            case EType::Bool:
            {
                bool& Value = static_cast<FFundamentalPropertyDesc<bool>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value);
            }
            break;
            case EType::String:
            {
                std::string& Value = static_cast<FFundamentalPropertyDesc<std::string>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value);
            }
            break;
            case EType::WString:
            {
                std::wstring& Value = static_cast<FFundamentalPropertyDesc<std::wstring>*>(Prop)->Get(InObject);
                if (InValue.HasMember(Prop->Name))
                {
                    GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value);
                }
            }
            break;
            case EType::Vec2:
            {
                Vec2& Value = static_cast<FFundamentalPropertyDesc<Vec2>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value);
            }
            break;
            case EType::Vec3:
            {
                Vec3& Value = static_cast<FFundamentalPropertyDesc<Vec3>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value);
            }
            break;
            case EType::Vec4:
            {
                Vec4& Value = static_cast<FFundamentalPropertyDesc<Vec4>*>(Prop)->Get(InObject);
                GetObjectFromJson(InValue.FindMember(Prop->Name)->value, Value);
            }
            break;
            default:
                PatchStruct(Prop->TypeDesc, InObject, InValue.FindMember(Prop->Name)->value);
                break;
            }
        }
    }
}

void MJsonDeserializer::PatchContainer(void* InContainer, FContainerPropertyDesc* InContainerPropDesc, rapidjson::Value& InJsonValue, void* InObject)
{
    uint32 Num = static_cast<uint32>(InJsonValue.MemberCount());

    if (InContainerPropDesc->GetNum(InObject) > 0)
    {
        InContainerPropDesc->Clear(InObject);
    }

    InContainerPropDesc->Resize(InObject, Num);

    for (auto Iter = InJsonValue.MemberBegin(); Iter != InJsonValue.MemberEnd(); ++Iter)
    {
        uint32 Index = static_cast<uint32>(std::stoi(Iter->name.GetString()));

        if (InContainerPropDesc->TypeDesc == nullptr)
        {
            switch (InContainerPropDesc->Type)
            {
            case EType::Int:
            case EType::Enum:
            {
                int* Value = static_cast<int*>(InContainerPropDesc->Get(InObject, Index));
                GetObjectFromJson(Iter->value, *Value);
            }
            break;
            case EType::Float:
            {
                float* Value = static_cast<float*>(InContainerPropDesc->Get(InObject, Index));
                GetObjectFromJson(Iter->value, *Value);
            }
            break;
            case EType::Vec2:
            {
                Vec2* Value = static_cast<Vec2*>(InContainerPropDesc->Get(InObject, Index));
                GetObjectFromJson(Iter->value, *Value);
            }
            break;
            case EType::Vec3:
            {
                Vec3* Value = static_cast<Vec3*>(InContainerPropDesc->Get(InObject, Index));
                GetObjectFromJson(Iter->value, *Value);
            }
            break;
            case EType::Vec4:
            {
                Vec4* Value = static_cast<Vec4*>(InContainerPropDesc->Get(InObject, Index));
                GetObjectFromJson(Iter->value, *Value);
            }
            break;
            case EType::String:
            {
                std::string* Value = static_cast<std::string*>(InContainerPropDesc->Get(InObject, Index));
                GetObjectFromJson(Iter->value, *Value);
            }
            break;
            case EType::WString:
            {
                std::wstring* Value = static_cast<std::wstring*>(InContainerPropDesc->Get(InObject, Index));
                GetObjectFromJson(Iter->value, *Value);
            }
            break;
            }
        }
        else
        {
            PatchStruct(InContainerPropDesc->TypeDesc, InContainerPropDesc->Get(InObject, Index), Iter->value);
        }
    }
}
