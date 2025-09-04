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
        else if (Prop->IsArray()) // 배열
        {
            OutValue.AddMember(PropNameValue, DispatchArray(Prop, InData), Allocator);
        }
        else
        {
            if (Prop->Type != EType::None)
            {
                OutValue.AddMember(PropNameValue, HandleData(Prop->Type, Prop->GetAsVoid(InData)), Allocator);
            }
            else
            {
                OutValue.AddMember(PropNameValue, HandleData(Prop->TypeDesc, Prop->GetAsVoid(InData), Prop->bSharedValue), Allocator);
            }
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

    for (size_t i = 0; i < Num; ++i)
    {
        if (InContainerPropDesc->Type != EType::None)
        {
            OutValue.AddMember(ToJsonValue(std::to_string(i)), HandleData(InContainerPropDesc->Type, InContainerPropDesc->Get(InData, i)), Allocator);
        }
        else
        {
            OutValue.AddMember(ToJsonValue(std::to_string(i)), HandleData(InContainerPropDesc->TypeDesc, InContainerPropDesc->Get(InData, i), InContainerPropDesc->bSharedValue), Allocator);
        }
    }

 //   {
 //       for (size_t i = 0; i < Num; ++i)
 //       {
 //           const void* Data = InContainerPropDesc->Get(InData, i);
 //           rapidjson::Value IndexValue = ToJsonValue(std::to_string(i));

 //           if (InContainerPropDesc->IsA<MAsset>())
 //           {
 //               rapidjson::Value AssetValue(kObjectType);

 //               std::shared_ptr<MAsset> Asset = *static_cast<const std::shared_ptr<MAsset>*>(Data);

 //               AssetValue.AddMember(ToJsonValue(MAsset::GetTypeDescStatic()->Name), DispatchStruct(MAsset::GetTypeDescStatic(), Asset.get()), Allocator);
 //               OutValue.AddMember(IndexValue, AssetValue, Allocator);
 //           }
 //           else if (InContainerPropDesc->IsA<MObject>())
 //           {
 //               if (InContainerPropDesc->bSharedValue)
 //               {
 //                   std::shared_ptr<MObject> Object = *static_cast<const std::shared_ptr<MObject>*>(Data);
 //                   OutValue.AddMember(IndexValue, DispatchStruct(InContainerPropDesc->TypeDesc, Object.get()), Allocator);
 //               }
 //               else
 //               {
 //                   OutValue.AddMember(IndexValue, DispatchStruct(InContainerPropDesc->TypeDesc, Data), Allocator);
 //               }
 //           }
 //           else
 //           {
 //               OutValue.AddMember(IndexValue, DispatchStruct(InContainerPropDesc->TypeDesc, Data), Allocator);
 //           }
 //       }
	//}

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

rapidjson::Value MJsonSerializer::DispatchArray(FPropertyDesc* InArrayPropDesc, const void* InData)
{
    rapidjson::Value OutValue(kArrayType);

    size_t Num = InArrayPropDesc->Num;
    for (size_t i = 0; i < Num; ++i)
    {
        if (InArrayPropDesc->Type != EType::None)
        {
            OutValue.PushBack(HandleData(InArrayPropDesc->Type, InArrayPropDesc->GetAsVoid(InData, i)), Allocator);
        }
        else
        {
            OutValue.PushBack(HandleData(InArrayPropDesc->TypeDesc, InArrayPropDesc->GetAsVoid(InData, i)), Allocator);
        }
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

rapidjson::Value MJsonSerializer::HandleData(const FTypeDesc* InTypeDesc, const void* InData, bool bShared)
{
    // InTypeDesc가 유효할 지 의문이 듬.

    rapidjson::Value OutValue(kObjectType);

    if (InTypeDesc->IsA<MAsset>())
    {
        std::shared_ptr<const MAsset> Asset = *static_cast<const std::shared_ptr<MAsset>*>(InData);

        rapidjson::Value AssetValue(kObjectType);
        AssetValue.AddMember(ToJsonValue(MAsset::GetTypeDescStatic()->Name), DispatchStruct(MAsset::GetTypeDescStatic(), Asset.get()), Allocator);
        OutValue = AssetValue;
    }
    else if (InTypeDesc->IsA<MObject>())
    {
        const MObject* Object = nullptr;
        if (bShared)
        {
            std::shared_ptr<const MObject> SharedObject = *static_cast<const std::shared_ptr<MObject>*>(InData);
            OutValue = DispatchStruct(InTypeDesc, SharedObject.get());
        }
        else
        {
            OutValue = DispatchStruct(InTypeDesc, InData);
        }
    }
    else
    {
        OutValue = DispatchStruct(InTypeDesc, InData);
    }

    return OutValue;
}
