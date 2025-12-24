#include "FileSystem.h"

namespace fs = std::filesystem;

fs::path MFIleSystem::RootPath = fs::current_path();

MFIleSystem::MFIleSystem()
{
    fs::recursive_directory_iterator Iter(RootPath);
    while (Iter != fs::end(Iter))
    {
        const fs::directory_entry& Entry = *Iter;
        std::cout << Entry.path() << std::endl;
        Iter++;
    }
}

std::wstring MFIleSystem::RootStr = MFIleSystem::RootPath.wstring();

std::wstring MFIleSystem::AbsolutePath(const std::wstring& InRelativePath)
{
    fs::path Path(InRelativePath);
    Path = Path.make_preferred();

    return AbsolutePath(Path);
}

fs::path MFIleSystem::AbsolutePath(const fs::path InRelativePath)
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
    fs::path Path(InPath);
    return Path.remove_filename().wstring();
}

std::wstring MFIleSystem::RelativePath(const std::wstring& InPath)
{
    fs::path Path(InPath);
    return RelativePath(Path);
}

std::wstring MFIleSystem::RelativePath(const fs::path& InPath)
{
    return std::move(fs::relative(InPath, MFIleSystem::RootPath).wstring());
}
