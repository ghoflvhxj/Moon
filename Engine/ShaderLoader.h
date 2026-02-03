#pragma once

#include "Include.h"
#include "Core/FileSystem.h"

#include "Module/Graphic/Shader/Shader.h"

class MShaderManager;
class FileFinder;

constexpr wchar_t* SHADER_DIRECTORY = TEXT("Shader");
constexpr wchar_t* SHADER_TYPE_DIR[] = {
    TEXT("Vertex"),
    TEXT("Pixel"),
    TEXT("Geometry"),
    TEXT("Compute"),
};

inline wchar_t* ShaderTypeDir(EShaderType InShaderType)
{
    return SHADER_TYPE_DIR[EnumToIndex(InShaderType)];
}

inline void GetResourceDirectory(WCHAR buffer[])
{
    GetCurrentDirectory(MAX_PATH, buffer);
    PathCombine(buffer, buffer, RESOURCE_DIRECTORY);
}

inline void getShaderDirectory(WCHAR buffer[])
{
    GetResourceDirectory(buffer);
    PathCombine(buffer, buffer, SHADER_DIRECTORY);
}

inline void getVertexShaderDirectory(WCHAR buffer[])
{
    getShaderDirectory(buffer);
    PathCombine(buffer, buffer, ShaderTypeDir(EShaderType::Vertex));
}

inline void getPixelShaderDirectory(WCHAR buffer[])
{
    getShaderDirectory(buffer);
    PathCombine(buffer, buffer, ShaderTypeDir(EShaderType::Pixel));
}

inline void getGeometryShaderDirectory(WCHAR buffer[])
{
    getShaderDirectory(buffer);
    PathCombine(buffer, buffer, ShaderTypeDir(EShaderType::Geometry));
}

//inline const std::wstring& GetResourceDirectory()
//{
//    return MFileSystem::ResourcePathStr;
//}

inline std::wstring GetShaderDirectory(EShaderType InShaderType)
{
    static std::filesystem::path ShaderPath = std::filesystem::path(MFileSystem::GetResourcePath()) / SHADER_DIRECTORY / ShaderTypeDir(InShaderType);
    return ShaderPath;
}

inline std::wstring GetComputeShaderDirectory()
{
    return GetShaderDirectory(EShaderType::Compute);
}

class ShaderLoader
{
public:
	bool loadShaderFiles(const std::unique_ptr<MShaderManager>& shaderManager);
private:
	bool loadVertexShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager);
	bool loadPixelShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager);
	bool loadGeometryShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager);
    bool LoadComputeShaders(const std::unique_ptr<MShaderManager>& shaderManager);
};
