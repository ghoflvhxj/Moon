#include "ResourceManager.h"
#include "ResourceLoader.h"

#include "Core/Asset.h"
#include "Mesh/DynamicMesh/DynamicMesh.h"

MResourceManager::MResourceManager()
{
    AddLoader(std::make_shared<MTextureLoader>());
    AddLoader(std::make_shared<MMeshLoader>());
    AddLoader(std::make_shared<MDynamicMeshLoader>());
    AddLoader(std::make_shared<MMaterialLoader>());
}

std::shared_ptr<MAsset> MResourceManager::Load(const std::wstring& InPath, const FTypeDesc* InTypeDesc)
{
    std::filesystem::path Path(InPath);
    Path = Path.make_preferred();

    if (Path.empty())
    {
        return nullptr;
    }

    if (Path.is_absolute() == false)
    {
        Path = MFileSystem::AbsolutePath(Path);
    }

    if (std::filesystem::exists(Path) == false)
    {
        std::wstring Msg = TEXT("파일이 없음: ") + Path.wstring();
        MSGBOX(Msg);
        return nullptr;
    }

    if (ResourceLoaders2.find(InTypeDesc) != ResourceLoaders2.end())
    {
        return ResourceLoaders2[InTypeDesc]->TryLoad(Path);
    }
    else
    {
        const std::wstring& FileExtension = Path.extension().wstring();
        if (ResourceLoaders.find(FileExtension) != ResourceLoaders.end())
        {
            return ResourceLoaders[FileExtension]->TryLoad(Path);
        }

        // 피직스 임시
        std::shared_ptr<MAsset> Asset(static_cast<MAsset*>(CreateObject(InTypeDesc)));
        Asset->LoadFromDisk(Path);
        //Asset->SetAssetPath(Path); 애셋 자체에 이미 Path가 있으니 읽어오게 하면 됨
        TempCache.emplace(Path, Asset);

        return TempCache[Path];
    }

    return nullptr;
}

void MResourceManager::AddLoader(const std::shared_ptr<MResourceLoader>& InLoader)
{
	for (const std::wstring& Extension : InLoader->GetExtensions())
	{
#if _DEBUG
		if (ResourceLoaders.find(Extension) != ResourceLoaders.end())
		{
			MSGBOX(TEXT("확장자의 리소스 로더가 덮어 씌워짐(") + Extension + TEXT(")"));
		}
#endif
		ResourceLoaders[Extension] = InLoader;
	}

    if (const FTypeDesc* TypeDesc = InLoader->GetSupportType())
    {
        ResourceLoaders2[TypeDesc] = InLoader;
    }
}

void MResourceManager::Release()
{
	ResourceLoaders.clear();
    ResourceLoaders2.clear();
}

std::shared_ptr<MAsset> MResourceManager::FindAsset(const std::wstring& InPath)
{
    std::filesystem::path Path(InPath);
    Path = Path.make_preferred();

    if (Path.empty())
    {
        return nullptr;
    }

    if (Path.is_absolute() == false)
    {
        Path = MFileSystem::AbsolutePath(Path);
    }

    for (auto& [Extension, ResourceLoader]: ResourceLoaders)
    {
        for (auto& [AssetPath, Asset] : ResourceLoader->GetLoadedResources())
        {
            if (AssetPath == Path)
            {
                return Asset;
            }
        }
    }

    for (auto& [TypeDesc, ResourceLoader] : ResourceLoaders2)
    {
        for (auto& [AssetPath, Asset] : ResourceLoader->GetLoadedResources())
        {
            if (AssetPath == Path)
            {
                return Asset;
            }
        }
    }

    auto& Iter = TempCache.find(Path);
    if (Iter != TempCache.end())
    {
        return Iter->second;
    }

    return nullptr;
}

std::shared_ptr<DynamicMesh> MResourceManager::FindDynamicMesh(const std::vector<FJoint> Joints)
{
    for (auto& [Path, Resource] : ResourceLoaders2[DynamicMesh::GetTypeDescStatic()]->GetLoadedResources())
    {
        auto& DM = Resource->CastToShared<DynamicMesh>();
        if (auto& SK = DM->GetSkeleton())
        {
            if (SK->GetJointNum() != GetSize(Joints))
            {
                continue;
            }

            if (SK->GetJoint(0).Name != Joints[0].Name)
            {
                continue;
            }

            return DM;
        }
    }

    return nullptr;
}
