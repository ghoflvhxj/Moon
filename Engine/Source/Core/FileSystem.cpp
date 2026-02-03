#include "FileSystem.h"

namespace fs = std::filesystem;

MFileSystem::MFileSystem()
{
    fs::recursive_directory_iterator Iter(GetRootPath());
    while (Iter != fs::end(Iter))
    {
        const fs::directory_entry& Entry = *Iter;
        std::cout << Entry.path() << std::endl;
        Iter++;
    }
}

const std::filesystem::path& MFileSystem::GetRootPath()
{
    static fs::path RootPath = fs::current_path();
    return RootPath;
}

const std::filesystem::path& MFileSystem::GetResourcePath()
{
    static fs::path ResourcePath = fs::path(GetRootPath()) / TEXT("Resources");
    return ResourcePath;
}

std::vector<std::wstring> MFileSystem::GetFiles(const std::wstring& InPath)
{
    std::vector<std::wstring> Files;

    fs::recursive_directory_iterator Iter(InPath);
    while (Iter != fs::end(Iter))
    {
        const fs::directory_entry& Entry = *Iter;
        Files.push_back(Entry.path());
        Iter++;
    }

    return Files;
}

std::wstring MFileSystem::AbsolutePath(const std::wstring& InRelativePath)
{
    fs::path Path(InRelativePath);
    Path = Path.make_preferred();

    return AbsolutePath(Path);
}

fs::path MFileSystem::AbsolutePath(const fs::path InRelativePath)
{
    if (InRelativePath.is_absolute() == false)
    {
        return GetRootPath() / InRelativePath;
    }
    else
    {
        return InRelativePath;
    }
}

std::wstring MFileSystem::GetDirectory(const std::wstring& InPath)
{
    fs::path Path(InPath);
    return Path.remove_filename().wstring();
}

std::wstring MFileSystem::GetFileName(const std::wstring& InPath, bool bIncludeExtension)
{
    fs::path FileName = fs::path(InPath).filename();

    if (bIncludeExtension == false)
    {
        FileName = FileName.replace_extension();
    }

    return FileName;
}

std::wstring MFileSystem::RelativePath(const std::wstring& InPath)
{
    fs::path Path(InPath);
    return RelativePath(Path);
}

std::wstring MFileSystem::RelativePath(const fs::path& InPath)
{
    return std::move(fs::relative(InPath, GetRootPath()).wstring());
}

bool MFileSystem::IsExist(const std::wstring& InPath)
{
    return fs::exists(AbsolutePath(InPath));
}

bool MFileSystem::HasExtension(const std::wstring& InPath)
{
    fs::path Path(InPath);
    return Path.has_extension();
}

bool MFileSystem::IsExtension(const std::wstring& InPath, const std::wstring& InExtension)
{
    if (HasExtension(InPath))
    {
        return fs::path(InPath).extension().wstring() == InExtension;
    }

    return false;
}

std::wstring MFileSystem::ReplaceExtension(const std::wstring& InPath, const std::wstring& InExtension)
{
    fs::path Path(InPath);
    return Path.replace_extension(InExtension).wstring();
}
