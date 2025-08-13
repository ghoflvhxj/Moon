#include "FileSystem.h"

std::filesystem::path MFIleSystem::RootPath = std::filesystem::current_path();
std::wstring MFIleSystem::RootStr = MFIleSystem::RootPath.wstring();

std::wstring MFIleSystem::AbsolutePath(const std::wstring& InRelativePath)
{
    std::filesystem::path Path(InRelativePath);
    Path = Path.make_preferred();

    return AbsolutePath(Path);
}

std::filesystem::path MFIleSystem::AbsolutePath(const std::filesystem::path InRelativePath)
{
    if (InRelativePath.is_absolute() == false)
    {
        return RootPath / InRelativePath;
    }
    else
    {
        return InRelativePath;
    }
}

std::wstring MFIleSystem::GetDirectory(const std::wstring& InPath)
{
    std::filesystem::path Path(InPath);
    return Path.remove_filename().wstring();
}

std::wstring MFIleSystem::RelativePath(const std::wstring& InPath)
{
    std::filesystem::path Path(InPath);
    return RelativePath(Path);
}

std::wstring MFIleSystem::RelativePath(const std::filesystem::path& InPath)
{
    return std::move(std::filesystem::relative(InPath, MFIleSystem::RootPath).wstring());
}
