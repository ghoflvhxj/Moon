#include "JsonDeserializer.h"

#include "MoonEngine.h"
#include "Core/Asset.h"
#include "Core/ResourceManager.h"

using namespace rapidjson;

MJsonDeserializer::MJsonDeserializer()
    : Allocator(Doc.GetAllocator())
{
}

void MJsonDeserializer::PatchTypeDesc(const FTypeDesc* InTypeDesc, void* InObject, rapidjson::Value& InValue)
{
	if (InTypeDesc == nullptr)
	{
		return;
	}

	for (auto& Prop : InTypeDesc->Properties)
	{
		if (InValue.HasMember(Prop->Name) == false)
		{
			continue;
		}

		rapidjson::Value& PropValue = InValue.FindMember(Prop->Name)->value;

		if (Prop->IsContainer(EContainerType::Vector))
		{
			PatchVector(static_cast<FVectorPropertyDesc*>(Prop), PropValue, InObject);
		}
		else if (Prop->IsContainer(EContainerType::Unordered_map))
		{
			PatchMap(static_cast<FMapPropertyDesc*>(Prop), PropValue, InObject);
		}
		else if (Prop->IsArray())
		{
			PatchArray(Prop, PropValue, InObject);
		}
		else        
		{
			void* Data = ReadData(Prop, PropValue);
			Prop->SetAsVoid(InObject, Data);

            if (Data != nullptr)
            {
                DeleteData(Prop, Data);
            }
		}
	}
}

void MJsonDeserializer::PatchVector(FVectorPropertyDesc* InContainerPropDesc, rapidjson::Value& InJsonValue, void* InObject)
{
	// 벡터는 객체 형태로 저장되있음. 
	// 값        -> "MyVector" : { "0" : 1, ... }
	// 오브젝트  -> "MyVector" : { "0" : {}, ... }
	//std::wstring Str = TEXT("컨테이너 ") + StringToWString(InContainerPropDesc->Name.data()) + TEXT(" 을(를) 읽는 중...\r\n");
	//LOG(Str);

	uint32 Num = static_cast<uint32>(InJsonValue.MemberCount());

	if (InContainerPropDesc->GetNum(InObject) > 0)
	{
		InContainerPropDesc->Clear(InObject);
		InContainerPropDesc->Reserve(InObject, Num);
	}

	for (auto Iter = InJsonValue.MemberBegin(); Iter != InJsonValue.MemberEnd(); ++Iter)
	{
		rapidjson::Value& VectorElement = Iter->value;
		void* Data = ReadData(InContainerPropDesc, VectorElement);

		InContainerPropDesc->PushBack(InObject, Data);

        if (Data != nullptr)
        {
            DeleteData(InContainerPropDesc, Data);
        }
	}
}

void MJsonDeserializer::PatchMap(FMapPropertyDesc* InContainerPropDesc, rapidjson::Value& InJsonValue, void* InObject)
{
	// 맵은 Pair 객체의 Array 형태로 저장되있음. 
	// "MyMap" : [ {"Key" : {}, "Value" : {} }, ... ]
	rapidjson::SizeType Num = InJsonValue.Size();
	for (rapidjson::SizeType Index = 0; Index < Num; ++Index)
	{
		rapidjson::Value& JsonKey = InJsonValue[Index].FindMember("Key")->value;
		rapidjson::Value& JsonValue = InJsonValue[Index].FindMember("Value")->value;

		void* KeyPtr = InContainerPropDesc->GetKeyInstance();
		if (InContainerPropDesc->ContainerKeyType != EType::None)
		{
			ReadFundamentalData(InContainerPropDesc->ContainerKeyType, JsonKey, KeyPtr);
		}
		else
		{
			KeyPtr = ReadCustomData(JsonKey, InContainerPropDesc->KeyTypeDesc);
		}

		void* ValuePtr = InContainerPropDesc->GetInstance();
		if (InContainerPropDesc->Type != EType::None)
		{
			ReadFundamentalData(InContainerPropDesc->Type, JsonValue, ValuePtr);
		}
		else
		{
			ValuePtr = ReadCustomData(JsonValue, InContainerPropDesc->TypeDesc, InContainerPropDesc->bSharedPtr);
		}

		if (KeyPtr != nullptr && ValuePtr != nullptr)
		{
			InContainerPropDesc->Set(InObject, KeyPtr, ValuePtr);
		}
	}
}

void MJsonDeserializer::PatchArray(FPropertyDesc* InPropertyDesc, rapidjson::Value& InJsonValue, void* InObject)
{
	rapidjson::SizeType Num = InJsonValue.Size();
	for (rapidjson::SizeType Index = 0; Index < Num; ++Index)
	{
		rapidjson::Value& JsonValue = InJsonValue[Index];
		void* Data = ReadData(InPropertyDesc, JsonValue);
		InPropertyDesc->SetAsVoid(InObject, Data, Index);

        if (Data != nullptr)
        {
            DeleteData(InPropertyDesc, Data);
        }
	}
}

void* MJsonDeserializer::ReadCustomData(rapidjson::Value& InValue, const FTypeDesc* InTypeDesc, bool bShared)
{
	// 리플렉션에 등록된 포인터 멤버가 있을때 디시리얼라이즈로 읽어 들이면 어떻게 처리해야 할까?
	if (InTypeDesc != nullptr)
	{
		if (InTypeDesc->IsA<MAsset>())
		{
			const FTypeDesc* AssetTypeDesc = MAsset::GetTypeDescStatic();
			if (InValue.HasMember(AssetTypeDesc->Name))
			{
				// 경로만 저장된 Asset을 읽어오고, 실제 타입을 이용해 로드함
				MAsset TempAsset;
				void* TempAssetPtr = &TempAsset;
				PatchTypeDesc(AssetTypeDesc, TempAssetPtr, InValue.FindMember(AssetTypeDesc->Name)->value);

				Test = g_ResourceManager->Load(TempAsset.GetAssetPath(), InTypeDesc);
				return Test != nullptr ? &Test : nullptr;
			}
		}
		else if (InTypeDesc->IsA<MObject>())
		{
			// 오브젝트면 클래스의 이름을 통해 실제 TypeDesc를 찾고, 인스턴스를 만들어 내야 함
			auto JsonMemIter = InValue.MemberBegin();
			if (JsonMemIter == InValue.MemberEnd())
			{
				return nullptr;
			}

			// 클래스 이름으로 TypeDesc를 찾음
			const FTypeDesc* TypeDesc = nullptr;
			const std::string& ClassName = JsonMemIter->name.GetString();
			if (GetTypeDescs().find(ClassName) != GetTypeDescs().end())
			{
				TypeDesc = GetTypeDescs()[ClassName];
			}
			// 프로퍼티 이름으로 프로퍼티를 찾아 TypeDesc를 구함
			else
			{
				const std::string& PropertyName = JsonMemIter->name.GetString();
				for (auto Prop : InTypeDesc->Properties)
				{
					if (Prop->Name == PropertyName)
					{
						TypeDesc = Prop->TypeDesc;
						break;
					}
				}
			}

			if (TypeDesc == nullptr)
			{
				return nullptr;
			}

			if (MObject* OutData = static_cast<MObject*>(CreateObject(TypeDesc)))
			{
				while (TypeDesc)
				{
					if (InValue.HasMember(TypeDesc->Name))
					{
						PatchTypeDesc(TypeDesc, OutData, InValue.FindMember(TypeDesc->Name)->value);
					}

					TypeDesc = TypeDesc->Parent;
				}

				if (bShared)
				{
					Test = std::shared_ptr<MObject>(OutData);
					Test->OnLoaded();
					return &Test;
				}
				else
				{
					return OutData;
				}
			}
		}
	}

	if (void* OutData = CreateData(InTypeDesc))
	{
		PatchTypeDesc(InTypeDesc, OutData, InValue);
		return OutData;
	}

	return nullptr;
}

void MJsonDeserializer::DeleteData(FPropertyDesc* InProp, void* InData)
{
    /*********************************************
    멤버 타입에 따라 다르게 처리해야 함

    커스텀 타입          ->  삭제 해야함.
    일반 타입            ->  이거는 인스턴스를 이용했기 때문에 삭제하면 안됨

    커스텀 타입 포인터   ->  포인터는 아직...
    일반 타입 포인터     ->  포인터는 아직...

    스마트 포인터        ->  삭제하면 안됨. ex) 액터의 컴포넌트 멤버
    **********************************************/
    
    assert(InData);

    if (InProp->IsCustomType() && InProp->bSharedPtr == false && InData != InProp->GetInstance())
    {
        InProp->Delete(InData);
    }
}

void* MJsonDeserializer::ReadData(FPropertyDesc* Prop, rapidjson::Value& PropValue)
{
	void* Data = nullptr;
	if (Prop->IsCustomType())
	{
		Data = ReadCustomData(PropValue, Prop->TypeDesc, Prop->bSharedPtr);
	}
	else
	{
		Data = Prop->GetInstance();
		ReadFundamentalData(Prop->Type, PropValue, Data);
	}

	return Data;
}

void MJsonDeserializer::ReadFundamentalData(EType InType, rapidjson::Value& InValue, void* InData)
{
	assert(InData);

	switch (InType)
	{
	case EType::Int:
	case EType::Enum:
	{
		GetObjectFromJson<int>(InValue, InData);
	}
	break;
	case EType::Float:
	{
		GetObjectFromJson<float>(InValue, InData);
	}
	break;
	case EType::Vec2:
	{
		GetObjectFromJson<Vec2>(InValue, InData);
	}
	break;
	case EType::Vec3:
	{
		GetObjectFromJson<Vec3>(InValue, InData);
	}
	break;
	case EType::Vec4:
	{
		GetObjectFromJson<Vec4>(InValue, InData);
	}
	break;
	case EType::Mat4:
	{
		GetObjectFromJson<Mat4>(InValue, InData);
	}
	break;
	case EType::String:
	{
		GetObjectFromJson<std::string>(InValue, InData);
	}
	break;
	case EType::WString:
	{
		GetObjectFromJson<std::wstring>(InValue, InData);
	}
	break;
	case EType::Bool:
	{
		GetObjectFromJson<bool>(InValue, InData);
	}
	break;
	}
}
