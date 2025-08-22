#include "JsonSerializer.h"
#include "Serializable.h"
#include "Core/Asset.h"

using namespace rapidjson;

MJsonSerializer::MJsonSerializer()
    : Allocator(Doc.GetAllocator())
{
    Doc.SetObject();
}

rapidjson::Value MJsonSerializer::DispatchStruct(const FTypeDesc* InTypeDesc, const void* InData)
{
    std::wstring Str = TEXT("구조체 ") + StringToWString(InTypeDesc->Name.data()) + TEXT(" 을(를) 읽는 중");
    LOG(Str);

	rapidjson::Value OutValue(kObjectType);

	if (InData == nullptr)
	{
		return OutValue;
	}

    for (auto& Prop : InTypeDesc->Properties)
    {
        rapidjson::Value PropNameValue = ToJsonValue(Prop->Name);

        if (Prop->IsContainer())	// 컨테이너
        {
            switch (Prop->ContainerType)
            {
                case EContainerType::Vector:
                {
                    OutValue.AddMember(PropNameValue, DispatchVector(static_cast<FVectorPropertyDesc*>(Prop), InData), Allocator);
                }
                break;
                case EContainerType::Unordered_map:
                {
                    // [ { "Key" : { ... }, "Value" : { ... } }, ] 요런 형태로 저장되야 함
                    // 키 타입이 오브젝트인 경우는...?
                    OutValue.AddMember(PropNameValue, DispatchMap(static_cast<FMapPropertyDesc*>(Prop), InData), Allocator);
                }
                break;
            }
        }
        else if (Prop->Num > 1) // 배열
        {
            size_t Num = Prop->Num;
            switch (Prop->Type)
            {
            case EType::Int:
            case EType::Enum:
            {
				OutValue.AddMember(PropNameValue, ToJsonValue(static_cast<FFundamentalPropertyDesc<int*>*>(Prop)->Get(InData), Num), Allocator);
            }
            break;
            case EType::Float:
            {
				OutValue.AddMember(PropNameValue, ToJsonValue(static_cast<FFundamentalPropertyDesc<float*>*>(Prop)->Get(InData), Num), Allocator);
            }
            break;
            case EType::Vec2:
            {
				OutValue.AddMember(PropNameValue, ToJsonValue(static_cast<FFundamentalPropertyDesc<Vec2*>*>(Prop)->Get(InData), Num), Allocator);
            }
            break;
            case EType::Vec3:
            {
				OutValue.AddMember(PropNameValue, ToJsonValue(static_cast<FFundamentalPropertyDesc<Vec3*>*>(Prop)->Get(InData), Num), Allocator);
            }
            break;
            case EType::Vec4:
            {
				OutValue.AddMember(PropNameValue, ToJsonValue(static_cast<FFundamentalPropertyDesc<Vec4*>*>(Prop)->Get(InData), Num), Allocator);
            }
            break;
			case EType::String:
			{
				OutValue.AddMember(PropNameValue, ToJsonValue(static_cast<FFundamentalPropertyDesc<std::string*>*>(Prop)->Get(InData), Num), Allocator);
			}
			break;
			case EType::WString:
			{
				OutValue.AddMember(PropNameValue, ToJsonValue(static_cast<FFundamentalPropertyDesc<std::wstring*>*>(Prop)->Get(InData), Num), Allocator);
			}
			break;
			case EType::Bool:
			{
				OutValue.AddMember(PropNameValue, ToJsonValue(static_cast<FFundamentalPropertyDesc<bool*>*>(Prop)->Get(InData), Num), Allocator);
			}
			break;
            default:
            {
				rapidjson::Value Temp(kArrayType);
				for (size_t i = 0; i < Prop->Num; ++i)
				{
					uint64 Base = (uint64)Prop->GetAsVoid(InData);
					uint64 MemoryPos = Base + (Prop->GetSize() * i);
					Temp.PushBack(DispatchStruct(Prop->TypeDesc, (void*)MemoryPos), Allocator);
				}

				OutValue.AddMember(PropNameValue, Temp, Allocator);
            }
            break;
            }
        }
        else
        {
            if (Prop->Type != EType::None)
            {
                OutValue.AddMember(PropNameValue, HandleData(Prop->Type, Prop->GetAsVoid(InData)), Allocator);
            }
            else
            {
                OutValue.AddMember(PropNameValue, HandleData(Prop->TypeDesc, Prop->GetAsVoid(InData)), Allocator);
            }

            //switch (Prop->Type)
            //{
            //case EType::Int:
            //case EType::Enum:
            //{
            //    OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<int>*>(Prop)->Get(InData)), Allocator);
            //}
            //break;
            //case EType::Float:
            //{
            //    OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<float>*>(Prop)->Get(InData)), Allocator);
            //}
            //break;
            //case EType::Vec2:
            //{
            //    OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<Vec2>*>(Prop)->Get(InData)), Allocator);
            //}
            //break;
            //case EType::Vec3:
            //{
            //    OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<Vec3>*>(Prop)->Get(InData)), Allocator);
            //}
            //break;
            //case EType::Vec4:
            //{
            //    OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<Vec4>*>(Prop)->Get(InData)), Allocator);
            //}
            //break;
            //case EType::String:
            //{
            //    OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<std::string>*>(Prop)->Get(InData)), Allocator);
            //}
            //break;
            //case EType::WString:
            //{
            //    OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<std::wstring>*>(Prop)->Get(InData)), Allocator);
            //}
            //break;
            //case EType::Bool:
            //{
            //    OutValue.AddMember(rapidjson::Value(Prop->Name, Allocator), ToJsonValue(static_cast<FFundamentalPropertyDesc<bool>*>(Prop)->Get(InData)), Allocator);
            //}
            //break;
            //default:
            //{
            //    if (Prop->IsA<MAsset>())
            //    {
            //        rapidjson::Value AssetValue(kObjectType);
            //        AssetValue.AddMember(ToJsonValue(MAsset::GetTypeDescStatic()->Name), DispatchStruct(MAsset::GetTypeDescStatic(), Prop->GetAsVoid(InData)), Allocator);
            //        OutValue.AddMember(ToJsonValue(Prop->Name), AssetValue, Allocator);
            //    }
            //    else
            //    {
            //        OutValue.AddMember(ToJsonValue(Prop->Name), DispatchStruct(Prop->TypeDesc, Prop->GetAsVoid(InData)), Allocator);
            //    }
            //}
            //break;
            //}
        }
    }

	return OutValue;
}

rapidjson::Value MJsonSerializer::DispatchVector(FVectorPropertyDesc* InContainerPropDesc, const void* InData)
{
    std::wstring Str = TEXT("컨테이너 ") + StringToWString(InContainerPropDesc->Name.data()) + TEXT(" 을(를) 읽는 중");
    LOG(Str);

	rapidjson::Value OutValue(kObjectType);

	size_t Num = InContainerPropDesc->GetNum(InData);
	if (InContainerPropDesc->TypeDesc == nullptr)
	{
		switch (InContainerPropDesc->Type)
		{
		case EType::Int:
		case EType::Enum:
		{
			auto Value = (int*)InContainerPropDesc->Get(InData, 0);
			OutValue = ToJsonValue(Value, Num, true);
		}
		break;
		case EType::Float:
		{
			auto Value = (float*)InContainerPropDesc->Get(InData, 0);
			OutValue = ToJsonValue(Value, Num, true);
		}
		break;
		case EType::Vec2:
		{
			auto Value = (Vec2*)InContainerPropDesc->Get(InData, 0);
			OutValue = ToJsonValue(Value, Num, true);
		}
		break;
		case EType::Vec3:
		{
			auto Value = (Vec3*)InContainerPropDesc->Get(InData, 0);
			OutValue = ToJsonValue(Value, Num, true);
		}
		break;
		case EType::Vec4:
		{
			auto Value = (Vec4*)InContainerPropDesc->Get(InData, 0);
			OutValue = ToJsonValue(Value, Num, true);
		}
		break;
		case EType::String:
		{
			auto Value = (std::string*)InContainerPropDesc->Get(InData, 0);
			OutValue = ToJsonValue(Value, Num, true);
		}
		break;
		case EType::WString:
		{
			auto Value = (std::wstring*)InContainerPropDesc->Get(InData, 0);
			OutValue = ToJsonValue(Value, Num, true);
		}
		break;
		}
	}
	else
	{
        if (InContainerPropDesc->IsA<MAsset>())
        {
            for (size_t i = 0; i < Num; ++i)
            {
                rapidjson::Value AssetValue(kObjectType);
                AssetValue.AddMember(rapidjson::Value(MAsset::GetTypeDescStatic()->Name, Allocator), DispatchStruct(MAsset::GetTypeDescStatic(), InContainerPropDesc->Get(InData, i)), Allocator);
                OutValue.AddMember(rapidjson::Value(std::to_string(i), Allocator), AssetValue, Allocator);
            }
        }
        else
        {
            for (size_t i = 0; i < Num; ++i)
            {
                OutValue.AddMember(rapidjson::Value(std::to_string(i), Allocator), DispatchStruct(InContainerPropDesc->TypeDesc, InContainerPropDesc->Get(InData, i)), Allocator);
            }
        }
	}

	return OutValue;
}

rapidjson::Value MJsonSerializer::DispatchMap(FMapPropertyDesc* InContainerPropDesc, const void* InData)
{
    rapidjson::Value OutValue(rapidjson::kArrayType);

    std::vector<const void*> ContainerKeys = InContainerPropDesc->GetKeys(InData);
    for (auto& ContainerKey : ContainerKeys)
    {
        rapidjson::Value Pair(rapidjson::kObjectType);

        // 키
        rapidjson::Value Key(rapidjson::kObjectType);
        if (InContainerPropDesc->KeyTypeDesc == nullptr)
        {
            Key = HandleData(InContainerPropDesc->ContainerKeyType, ContainerKey);
        }
        else
        {
            // TODO. 오브젝트가 아닌 경우는 좀 더 생각해봐야 함
            if (InContainerPropDesc->IsA<MObject>())
            {
                const MObject* Object = reinterpret_cast<const MObject*>(ContainerKey);
                const FTypeDesc* Current = Object->GetTypeDesc();
                while (Current)
                {
                    Key.AddMember(ToJsonValue(Current->Name), DispatchStruct(Current, ContainerKey), Allocator);
                    Current = Current->Parent;
                }
            }
        }
        Pair.AddMember("Key", Key, Allocator);

        // 값
        rapidjson::Value Val(rapidjson::kObjectType);
        void* ValueData = InContainerPropDesc->Get(InData, ContainerKey);
        if (InContainerPropDesc->TypeDesc == nullptr)
        {
            Val = HandleData(InContainerPropDesc->Type, ValueData);
        }
        else
        {
            if (InContainerPropDesc->IsA<MObject>())
            {
                const MObject* Object = nullptr;
                if (InContainerPropDesc->bSharedValue)
                {
                    std::shared_ptr<MObject> Ptr = *static_cast<std::shared_ptr<MObject>*>(ValueData);
                    Object = Ptr.get();
                }
                else
                {
                    Object = static_cast<const MObject*>(ValueData);
                }
                
                if (Object == nullptr)
                {
                    continue;
                }

                const FTypeDesc* Current = Object->GetTypeDesc();
                while (Current)
                {
                    Val.AddMember(ToJsonValue(Current->Name), DispatchStruct(Current, Object), Allocator);
                    Current = Current->Parent;
                }
            }

        }
        Pair.AddMember("Value", Val, Allocator);

        OutValue.PushBack(Pair, Allocator);
    }


    return OutValue;
}

rapidjson::Value MJsonSerializer::HandleData(EType InType, const void* InData)
{
    rapidjson::Value OutValue(kObjectType);

    switch (InType)
    {
        case EType::Int:
        case EType::Enum:
        {
            OutValue = ToJsonValue(CastData<int>(InData));
        }
        break;
        case EType::Float:
        {
            OutValue = ToJsonValue(CastData<float>(InData));
        }
        break;
        case EType::Vec2:
        {
            OutValue = ToJsonValue(CastData<Vec2>(InData));
        }
        break;
        case EType::Vec3:
        {
            OutValue = ToJsonValue(CastData<Vec3>(InData));
        }
        break;
        case EType::Vec4:
        {
            OutValue = ToJsonValue(CastData<Vec4>(InData));
        }
        break;
        case EType::String:
        {
            OutValue = ToJsonValue(CastData<std::string>(InData));
        }
        break;
        case EType::WString:
        {
            OutValue = ToJsonValue(CastData<std::wstring>(InData));
        }
        break;
        case EType::Bool:
        {
            OutValue = ToJsonValue(CastData<bool>(InData));
        }
        break;
    }

    return OutValue;
}

rapidjson::Value MJsonSerializer::HandleData(const FTypeDesc* InTypeDesc, const void* InData)
{
    // InTypeDesc가 유효할 지 의문이 듬.

    rapidjson::Value OutValue(kObjectType);

    if (InTypeDesc->IsA<MAsset>())
    {
        rapidjson::Value AssetValue(kObjectType);
        AssetValue.AddMember(ToJsonValue(MAsset::GetTypeDescStatic()->Name), DispatchStruct(MAsset::GetTypeDescStatic(), InData), Allocator);
        OutValue = AssetValue;
    }
    else if (InTypeDesc->IsA<MObject>())
    {
        const MObject* Object = static_cast<const MObject*>(InData);
        OutValue = DispatchStruct(InTypeDesc, InData);
    }
    else
    {
        OutValue = DispatchStruct(InTypeDesc, InData);
    }

    return OutValue;
}
