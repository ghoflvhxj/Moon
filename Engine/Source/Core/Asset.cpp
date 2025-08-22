#include "Asset.h"

#include "FileSystem.h"

MAsset::~MAsset()
{
    std::wstring Msg = TEXT("Release Asset: ") + GetAssetPath();
    LOG(Msg);
}

void MAsset::LoadFromDisk(const std::wstring& InPath)
{
    SetAssetPath(InPath);

    Super::LoadFromDisk(InPath);
}

void MAsset::OnLoaded()
{
    std::wstring Msg = TEXT("Asset Loaded: ") + GetAssetPath();
    LOG(Msg);
}

void MAsset::SetAssetPath(const std::wstring& InPath)
{
    std::filesystem::path FileSystemPath(InPath);
    Path = FileSystemPath.is_absolute() ? MFIleSystem::RelativePath(FileSystemPath) : InPath;
}
