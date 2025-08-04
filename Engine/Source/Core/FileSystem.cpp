#include "FileSystem.h"

std::filesystem::path MFIleSystem::RootPath = std::filesystem::current_path();
std::wstring MFIleSystem::RootStr = MFIleSystem::RootPath.wstring();

std::wstring MFIleSystem::CombinePath(const std::wstring& InRelativePath)
{
    return RootStr + TEXT("/") + InRelativePath;
}

std::filesystem::path MFIleSystem::CombinePath(const std::filesystem::path InRelativePath)
{
    return RootPath / InRelativePath;
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
