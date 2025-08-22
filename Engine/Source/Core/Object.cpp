#include "Object.h"
#include "Asset.h"

#include "Core/Serialize/JsonDeserializer.h"
#include "Core/ResourceManager.h"

void MObject::LoadFromDisk(const std::wstring& InPath)
{
    if (Load(InPath))
    {
        OnLoaded();
    }
}

bool MObject::Load(const std::wstring& InPath)
{
    if (InPath.empty())
    {
        return false;
    }

    std::filesystem::path FileSystemPath(InPath);
    if (FileSystemPath.extension() == TEXT(".json"))
    {
        MJsonDeserializer Deserializer;
        Deserializer.Deserialize(shared_from_this(), InPath);
    }

    //// Asset인 멤버가 있으면 로딩.
    //const FTypeDesc* TypeDesc = GetTypeDesc();
    //for (FPropertyDesc* PropertyDesc : TypeDesc->Properties)
    //{
    //    if (PropertyDesc->IsA<MAsset>())
    //    {
    //        if (PropertyDesc->IsContainer())
    //        {
    //            FVectorPropertyDesc* ContainerPropertyDesc = static_cast<FVectorPropertyDesc*>(PropertyDesc);
    //            size_t Num = ContainerPropertyDesc->GetNum(this);
    //            for (size_t i = 0; i < Num; ++i)
    //            {
    //                if (MAsset* Asset = static_cast<MAsset*>(ContainerPropertyDesc->Get(this, i)))
    //                {
    //                    std::shared_ptr<MAsset> SharedAsset = g_ResourceManager->Load(Asset->GetAssetPath(), Asset->GetTypeDesc());
    //                    void* AssetPtr = &SharedAsset;
    //                    ContainerPropertyDesc->Set(this, i, AssetPtr);
    //                }
    //            }
    //        }
    //        else if (PropertyDesc->IsArray())
    //        {
    //            for (size_t i = 0; i < PropertyDesc->Num; ++i)
    //            {
    //                if (MAsset* Asset = static_cast<MAsset*>(PropertyDesc->GetAsVoid(this, i)))
    //                {
    //                    std::shared_ptr<MAsset> SharedAsset = g_ResourceManager->Load(Asset->GetAssetPath(), Asset->GetTypeDesc());
    //                    void* AssetPtr = &SharedAsset;
    //                    PropertyDesc->SetAsVoid(this, &SharedAsset, i);
    //                }
    //            }
    //        }
    //        else
    //        {
    //            if (MAsset* Asset = static_cast<MAsset*>(PropertyDesc->GetAsVoid(this)))
    //            {
    //                std::shared_ptr<MAsset> SharedAsset = g_ResourceManager->Load(Asset->GetAssetPath(), Asset->GetTypeDesc());
    //                void* AssetPtr = &SharedAsset;
    //                PropertyDesc->SetAsVoid(this, &SharedAsset);
    //            }
    //        }
    //    }
    //}

    return true;
}

void MObject::OnLoaded()
{
    // DoNothing
}
