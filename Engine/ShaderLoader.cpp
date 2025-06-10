#include "Include.h"
#include "ShaderLoader.h"

#include "ShaderManager.h"
#include "Shader.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "GeometryShader.h"

#include "FileFinder.h"

ShaderLoader::ShaderLoader()
{
}

ShaderLoader::~ShaderLoader()
{
}

const bool ShaderLoader::loadShaderFiles(const std::unique_ptr<MShaderManager>& shaderManager)
{
	// 멀티 쓰레드로 변경하기
	loadVertexShaderFromFiles(shaderManager);
	loadPixelShaderFromFiles(shaderManager);
	loadGeometryShaderFromFiles(shaderManager);

	return true;
}

const bool ShaderLoader::loadVertexShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager)
{
	OutputDebugString(TEXT("VertexShader 불러오는 중...\r\n"));

	WCHAR path[MAX_PATH] = {};
	getVertexShaderDirectory(path);
	FileFinder fileFinder(path);

	auto &fileList = fileFinder.getFileList();
	for (auto &filePathName : fileList) 
	{
		std::shared_ptr<VertexShader> pShader = std::make_shared<VertexShader>(filePathName);

		WCHAR fileName[MAX_PATH] = {};
		lstrcpy(fileName, PathFindFileName(filePathName.c_str()));

		shaderManager->addVertexShader(fileName, pShader);
	}

	return true;
}

const bool ShaderLoader::loadPixelShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager)
{
	OutputDebugString(TEXT("PixelShader 불러오는 중...\r\n"));

	WCHAR path[MAX_PATH];
	getPixelShaderDirectory(path);
	FileFinder fileFinder(path);

	auto &fileList = fileFinder.getFileList();
	for (auto &filePathName : fileList)
	{
		std::shared_ptr<PixelShader> pShader = std::make_shared<PixelShader>(filePathName);

		WCHAR fileName[MAX_PATH] = {};
		lstrcpy(fileName, PathFindFileName(filePathName.c_str()));

		shaderManager->addPixelShader(fileName, pShader);
	}

	return true;
}

const bool ShaderLoader::loadGeometryShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager)
{
	OutputDebugString(TEXT("GeometryShader 불러오는 중...\r\n"));

	WCHAR path[MAX_PATH] = {};
	getGeometryShaderDirectory(path);
	FileFinder fileFinder(path);

	auto &fileList = fileFinder.getFileList();
	for (auto &filePathName : fileList)
	{
		std::shared_ptr<GeometryShader> pShader = std::make_shared<GeometryShader>(filePathName);

		WCHAR fileName[MAX_PATH] = {};
		lstrcpy(fileName, PathFindFileName(filePathName.c_str()));

		shaderManager->addGeometryShader(fileName, pShader);
	}

	return true;
}
