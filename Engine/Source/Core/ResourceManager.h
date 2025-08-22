#pragma once

#include "Include.h"

#include "FileSystem.h"
#include "ResourceLoader.h"

class MAsset;

// AssetManager로 변경하기
class ENGINE_DLL MResourceManager
{
public:
    MResourceManager();
	~MResourceManager() = default;

public:
    //void Load(MAsset* InAsset)
    //{
    //    std::filesystem::path Path(InAsset->GetAssetPath());

    //    if (Path.empty())
    //    {
    //        return;
    //    }

    //    if (Path.is_absolute() == false)
    //    {
    //        Path = MFIleSystem::CombinePath(Path);
    //    }

    //    if (std::filesystem::exists(Path) == false)
    //    {
    //        std::wstring Msg = TEXT("파일이 없음: ") + Path.wstring();
    //        MSGBOX(Msg);
    //        return;
    //    }

    //    if (ResourceLoaders2.find(InTypeDesc) != ResourceLoaders2.end())
    //    {
    //        return ResourceLoaders2[InTypeDesc]->TryLoad(InPath);
    //    }
    //}

    std::shared_ptr<MAsset> Load(const std::wstring& InPath, const FTypeDesc* InTypeDesc);

	template <class T>
	bool Load(const std::wstring& InPath, std::shared_ptr<T>& OutResource)
	{
        std::filesystem::path Path(InPath);
        Path = Path.make_preferred();

        if (Path.empty())
        {
            return false;
        }

        if (Path.is_absolute() == false)
        {
            Path = MFIleSystem::AbsolutePath(Path);
        }

        if (std::filesystem::exists(Path) == false)
        {
            std::wstring Msg = TEXT("파일이 없음: ") + Path.wstring();
            MSGBOX(Msg);
            return false;
        }

        const FTypeDesc* TypeDesc = T::GetTypeDescStatic();
        if (TypeDesc == MAsset::GetTypeDescStatic())
        {
            TypeDesc = OutResource->GetTypeDesc();
        }

        if (ResourceLoaders2.find(TypeDesc) != ResourceLoaders2.end())
        {
            OutResource = std::static_pointer_cast<T>(ResourceLoaders2[TypeDesc]->TryLoad(Path));
        }
        else
        {
            const std::wstring& FileExtension = Path.extension().wstring();
            if (ResourceLoaders.find(FileExtension) == ResourceLoaders.end())
            {
                MSGBOX(TEXT("지원되지 않은 파일 확장자(") + FileExtension + TEXT(")"));
                return false;
            }

            OutResource = std::static_pointer_cast<T>(ResourceLoaders[FileExtension]->TryLoad(Path));
        }

		return OutResource != nullptr;
	}

	void AddLoader(const std::shared_ptr<MResourceLoader>& ResourceLoader);
	void Release();

protected:
    // 확장자, 리소스 로더 쌍의 맵
	std::map<std::wstring, std::shared_ptr<MResourceLoader>> ResourceLoaders;
    // TypeDesc, 리소스 로더 쌍의 맵
    std::map<const FTypeDesc*, std::shared_ptr<MResourceLoader>> ResourceLoaders2;

    // 아직 지원하지 않은 애셋들을 매니저가 직접 관리
    // TODO
    std::map<std::wstring, std::shared_ptr<MAsset>> TempCache;
};