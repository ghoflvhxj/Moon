#pragma once

#include "Include.h"

class ENGINE_DLL MFileSystem
{
public:
    MFileSystem();

public:
    static std::wstring RootStr;
    static std::filesystem::path RootPath;

    static std::wstring AbsolutePath(const std::wstring& InRelativePath);
    static std::filesystem::path AbsolutePath(const std::filesystem::path InRelativePath);

    // 파일 이름을 제거해, 파일이 위치한 디렉토리를 얻음
    static std::wstring GetDirectory(const std::wstring& InPath);

    // 상대 경로로 변경
    static std::wstring RelativePath(const std::wstring& InPath);
    static std::wstring RelativePath(const std::filesystem::path& InPath);

    static bool IsExist(const std::wstring& InPath);
};