#include "ShaderLoader.h"

#include "ShaderManager.h"
#include "Module/Graphic/Shader/Shader.h"
#include "Module/Graphic/Shader/VertexShader.h"
#include "Module/Graphic/Shader/PixelShader.h"
#include "Module/Graphic/Shader/GeometryShader.h"

#include "MoonEngine.h"
#include "GraphicDevice.h"

#include "FileFinder.h"

bool ShaderLoader::loadShaderFiles(const std::unique_ptr<MShaderManager>& shaderManager)
{
	// 멀티 쓰레드로 변경하기
	loadVertexShaderFromFiles(shaderManager);
	loadPixelShaderFromFiles(shaderManager);
	loadGeometryShaderFromFiles(shaderManager);
    LoadComputeShaders(shaderManager);

	return true;
}

bool ShaderLoader::loadVertexShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager)
{
	OutputDebugString(TEXT("VertexShader 불러오는 중...\r\n"));

	WCHAR path[MAX_PATH] = {};
	getVertexShaderDirectory(path);
	FileFinder fileFinder(path);

	auto &fileList = fileFinder.getFileList();
	for (auto &filePathName : fileList) 
	{
		std::shared_ptr<MVertexShader> pShader = std::make_shared<MVertexShader>(filePathName);

		WCHAR fileName[MAX_PATH] = {};
		lstrcpy(fileName, PathFindFileName(filePathName.c_str()));

		shaderManager->addVertexShader(fileName, pShader);
	}

	return true;
}

bool ShaderLoader::loadPixelShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager)
{
	OutputDebugString(TEXT("PixelShader 불러오는 중...\r\n"));

	WCHAR path[MAX_PATH];
	getPixelShaderDirectory(path);
	FileFinder fileFinder(path);

	auto &fileList = fileFinder.getFileList();
	for (auto &filePathName : fileList)
	{
		std::shared_ptr<MPixelShader> pShader = std::make_shared<MPixelShader>(filePathName);

		WCHAR fileName[MAX_PATH] = {};
		lstrcpy(fileName, PathFindFileName(filePathName.c_str()));

		shaderManager->addPixelShader(fileName, pShader);
	}

	return true;
}

bool ShaderLoader::loadGeometryShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager)
{
	OutputDebugString(TEXT("GeometryShader 불러오는 중...\r\n"));

	WCHAR path[MAX_PATH] = {};
	getGeometryShaderDirectory(path);
	FileFinder fileFinder(path);

	auto &fileList = fileFinder.getFileList();
	for (auto &filePathName : fileList)
	{
		std::shared_ptr<MGeometryShader> pShader = std::make_shared<MGeometryShader>(filePathName);

		WCHAR fileName[MAX_PATH] = {};
		lstrcpy(fileName, PathFindFileName(filePathName.c_str()));

		shaderManager->addGeometryShader(fileName, pShader);
	}

	return true;
}

bool ShaderLoader::LoadComputeShaders(const std::unique_ptr<MShaderManager>& shaderManager)
{
    OutputDebugString(TEXT("ComputeShader 불러오는 중...\r\n"));

    std::wstring Path = GetComputeShaderDirectory();
    auto& Files = MFileSystem::GetFiles(Path);
    for (auto& FilePath : Files)
    {
        MComputeShader ComputeShader = getGraphicDevice()->CreateComputeShader(FilePath);
        shaderManager->AddComputeShader(FilePath, ComputeShader);
    }

    return true;
}
