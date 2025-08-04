#include "Asset.h"

#include "FileSystem.h"

void MAsset::SetAssetPath(const std::wstring& InPath)
{
    std::filesystem::path FileSystemPath(InPath);
    Path = FileSystemPath.is_absolute() ? MFIleSystem::RelativePath(FileSystemPath) : InPath;
}
