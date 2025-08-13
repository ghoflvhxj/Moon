#include "Asset.h"

#include "FileSystem.h"
#include "Core/Serialize/JsonDeserializer.h"
#include "Core/ResourceManager.h"

void MAsset::LoadFromDisk(const std::wstring& InPath)
{
    SetAssetPath(InPath);
    if (Load())
    {
        OnLoaded();
    }
}

bool MAsset::Load()
{
    if (Path.empty())
    {
        return false;
    }

    std::filesystem::path FileSystemPath(Path);
    if (FileSystemPath.extension() == TEXT(".json"))
    {
        MJsonDeserializer Deserializer;
        Deserializer.Deserialize(shared_from_this(), Path);
    }

    // Asset인 멤버가 있으면 로딩.
    const FTypeDesc* TypeDesc = GetTypeDesc();
    for (FPropertyDesc* PropertyDesc : TypeDesc->Properties)
    {
        if (PropertyDesc->IsA<MAsset>())
        {
            if (PropertyDesc->IsContainer())
            {
                FContainerPropertyDesc* ContainerPropertyDesc = static_cast<FContainerPropertyDesc*>(PropertyDesc);
                size_t Num = ContainerPropertyDesc->GetNum(this);
                for (size_t i = 0; i < Num; ++i)
                {
                    if (MAsset* Asset = static_cast<MAsset*>(ContainerPropertyDesc->Get(this, i)))
                    {
                        std::shared_ptr<MAsset> SharedAsset = g_ResourceManager->Load(Asset->Path, Asset->GetTypeDesc());
                        ContainerPropertyDesc->Set(this, i, &SharedAsset);
                    }
                }
            }
            else if (PropertyDesc->IsArray())
            {
                for (size_t i = 0; i < PropertyDesc->Num; ++i)
                {
                    if (MAsset* Asset = static_cast<MAsset*>(PropertyDesc->GetAsVoid(this, i)))
                    {
                        std::shared_ptr<MAsset> SharedAsset = g_ResourceManager->Load(Asset->Path, Asset->GetTypeDesc());
                        PropertyDesc->SetAsVoid(this, &SharedAsset, i);
                    }
                }
            }
            else
            {
                if (MAsset* Asset = static_cast<MAsset*>(PropertyDesc->GetAsVoid(this)))
                {
                    std::shared_ptr<MAsset> SharedAsset = g_ResourceManager->Load(Asset->Path, Asset->GetTypeDesc());
                    PropertyDesc->SetAsVoid(this, &SharedAsset);
                }
            }


            // 멤버가 스마트 포인터인데, 스마트 포인터의 raw를 가져왔어
            // 멤버 스마트 포인터에, 리소스 매니저에 저장된 애셋 스마트 포인터를 대입해야 함
        }
    }

    return true;
}

void MAsset::OnLoaded()
{
    std::wstring Msg = TEXT("Loaded: ") + Path;
    OutputDebugStringW(Msg.c_str());
}

void MAsset::SetAssetPath(const std::wstring& InPath)
{
    std::filesystem::path FileSystemPath(InPath);
    Path = FileSystemPath.is_absolute() ? MFIleSystem::RelativePath(FileSystemPath) : InPath;
}
