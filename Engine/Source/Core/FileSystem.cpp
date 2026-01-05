#include "FileSystem.h"

namespace fs = std::filesystem;

fs::path MFileSystem::RootPath = fs::current_path();

MFileSystem::MFileSystem()
{
    fs::recursive_directory_iterator Iter(RootPath);
    while (Iter != fs::end(Iter))
    {
        const fs::directory_entry& Entry = *Iter;
        std::cout << Entry.path() << std::endl;
        Iter++;
    }
}

std::wstring MFileSystem::RootStr = MFileSystem::RootPath.wstring();

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
        return RootPath / InRelativePath;
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

std::wstring MFileSystem::RelativePath(const std::wstring& InPath)
{
    fs::path Path(InPath);
    return RelativePath(Path);
}

std::wstring MFileSystem::RelativePath(const fs::path& InPath)
{
    return std::move(fs::relative(InPath, MFileSystem::RootPath).wstring());
}

bool MFileSystem::IsExist(const std::wstring& InPath)
{
    return fs::exists(AbsolutePath(InPath));
}
