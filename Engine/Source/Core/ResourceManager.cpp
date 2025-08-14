#include "ResourceManager.h"
#include "ResourceLoader.h"

#include "Core/Asset.h"

MResourceManager::MResourceManager()
{
    AddLoader(std::make_shared<MTextureLoader>());
    AddLoader(std::make_shared<MMeshLoader>());
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
        Path = MFIleSystem::AbsolutePath(Path);
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

        MSGBOX(TEXT("지원되지 않은 파일 확장자(") + FileExtension + TEXT(")"));

        // 피직스 임시
        std::shared_ptr<MAsset> Asset = std::make_shared<MAsset>();
        Asset->SetAssetPath(InPath);
        return Asset;
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