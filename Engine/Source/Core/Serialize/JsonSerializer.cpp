#include "JsonSerializer.h"
#include "Serializable.h"

using namespace rapidjson;

MJsonSerializer::MJsonSerializer()
    : Allocator(Doc.GetAllocator())
{
    Doc.SetObject();
}

rapidjson::Value MJsonSerializer::DispatchStruct(const FTypeDesc* InTypeDesc, void* InObject)
{
	rapidjson::Value OutValue(kObjectType);

	if (InObject == nullptr)
	{
		return OutValue;
	}

    for (auto& Prop : InTypeDesc->Properties)
    {
        if (Prop->bContainer)	// 컨테이너
        {
			OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), DispatchContainer(static_cast<FContainerPropertyDesc*>(Prop), InObject), Allocator);
        }
        else if (Prop->Num > 1) // 배열
        {
			void* ArrayObject = Prop->GetAsVoid(InObject);
			uint32 Num = Prop->Num;
            switch (Prop->Type)
            {
            case EType::Int:
            case EType::Enum:
            {
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<int*>*>(Prop)->Get(ArrayObject), Num), Allocator);
            }
            break;
            case EType::Float:
            {
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<float*>*>(Prop)->Get(ArrayObject), Num), Allocator);
            }
            break;
            case EType::Vec2:
            {
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<Vec2*>*>(Prop)->Get(ArrayObject), Num), Allocator);
            }
            break;
            case EType::Vec3:
            {
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<Vec3*>*>(Prop)->Get(ArrayObject), Num), Allocator);
            }
            break;
            case EType::Vec4:
            {
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<Vec4*>*>(Prop)->Get(ArrayObject), Num), Allocator);
            }
            break;
			case EType::String:
			{
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<std::string*>*>(Prop)->Get(ArrayObject), Num), Allocator);
			}
			break;
			case EType::WString:
			{
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<std::wstring*>*>(Prop)->Get(ArrayObject), Num), Allocator);
			}
			break;
			case EType::Bool:
			{
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<bool*>*>(Prop)->Get(ArrayObject), Num), Allocator);
			}
			break;
            default:
            {
				for (size_t i = 0; i < Prop->Num; ++i)
				{
					OutValue.AddMember(rapidjson::Value(std::to_string(i), Allocator), DispatchStruct(Prop->TypeDesc, ArrayObject), Allocator);
				}
            }
            break;
            }
        }
        else
        {
            switch (Prop->Type)
            {
            case EType::Int:
            case EType::Enum:
            {
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<int>*>(Prop)->Get(InObject)), Allocator);
            }
            break;
            case EType::Float:
            {
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<float>*>(Prop)->Get(InObject)), Allocator);
            }
            break;
            case EType::Vec2:
            {
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<Vec2>*>(Prop)->Get(InObject)), Allocator);
            }
            break;
            case EType::Vec3:
            {
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<Vec3>*>(Prop)->Get(InObject)), Allocator);
            }
            break;
            case EType::Vec4:
            {
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<Vec4>*>(Prop)->Get(InObject)), Allocator);
            }
            break;
			case EType::String:
			{
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<std::string>*>(Prop)->Get(InObject)), Allocator);
			}
			break;
			case EType::WString:
			{
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<std::wstring>*>(Prop)->Get(InObject)), Allocator);
			}
			break;
			case EType::Bool:
			{
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<bool>*>(Prop)->Get(InObject)), Allocator);
			}
			break;
            default:
            {
				OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), DispatchStruct(Prop->TypeDesc, Prop->GetAsVoid(InObject)), Allocator);
            }
            break;
            }
        }
    }

	return OutValue;
}

rapidjson::Value MJsonSerializer::DispatchContainer(FContainerPropertyDesc* InContainerPropDesc, void* InObject)
{
	rapidjson::Value OutValue(kArrayType);

	size_t Num = InContainerPropDesc->GetNum(InObject);
	if (InContainerPropDesc->TypeDesc == nullptr)
	{
		switch (InContainerPropDesc->Type)
		{
		case EType::Int:
		case EType::Enum:
		{
			auto Value = (int*)InContainerPropDesc->Get(InObject, 0);
			OutValue = ToJsonValue(Value, Num);
		}
		break;
		case EType::Float:
		{
			auto Value = (float*)InContainerPropDesc->Get(InObject, 0);
			OutValue = ToJsonValue(Value, Num);
		}
		break;
		case EType::Vec2:
		{
			auto Value = (Vec2*)InContainerPropDesc->Get(InObject, 0);
			OutValue = ToJsonValue(Value, Num);
		}
		break;
		case EType::Vec3:
		{
			auto Value = (Vec3*)InContainerPropDesc->Get(InObject, 0);
			OutValue = ToJsonValue(Value, Num);
		}
		break;
		case EType::Vec4:
		{
			auto Value = (Vec4*)InContainerPropDesc->Get(InObject, 0);
			OutValue = ToJsonValue(Value, Num);
		}
		break;
		case EType::String:
		{
			auto Value = (std::string*)InContainerPropDesc->Get(InObject, 0);
			OutValue = ToJsonValue(Value, Num);
		}
		break;
		case EType::WString:
		{
			auto Value = (std::wstring*)InContainerPropDesc->Get(InObject, 0);
			OutValue = ToJsonValue(Value, Num);
		}
		break;
		}

		return OutValue;
	}
	else
	{
		rapidjson::Value OutValue(kObjectType);
		for (size_t i = 0; i < Num; ++i)
		{
			OutValue.AddMember(rapidjson::Value(std::to_string(i), Allocator), DispatchStruct(InContainerPropDesc->TypeDesc, InContainerPropDesc->Get(InObject, i)), Allocator);
		}

		return OutValue;
	}
}